#include "gpu.h"

#include "etc1.h"
#include "renderer_gl.h"
#include "shader.h"

#undef PTR
#define PTR(addr) ((void*) &gpu->mem[addr])

static const GLenum texfilter[2] = {GL_NEAREST, GL_LINEAR};
static const GLenum texwrap[4] = {
    GL_CLAMP_TO_EDGE,
    GL_CLAMP_TO_BORDER,
    GL_REPEAT,
    GL_MIRRORED_REPEAT,
};
static const GLenum depth_func[8] = {
    GL_NEVER, GL_ALWAYS, GL_EQUAL,   GL_NOTEQUAL,
    GL_LESS,  GL_LEQUAL, GL_GREATER, GL_GEQUAL,
};
static const GLenum prim_mode[4] = {
    GL_TRIANGLES,
    GL_TRIANGLE_STRIP,
    GL_TRIANGLE_FAN,
    GL_TRIANGLES,
};

u32 f24tof32(u32 i) {
    u32 sgn = (i >> 23) & 1;
    u32 exp = (i >> 16) & MASK(7);
    u32 mantissa = i & 0xffff;
    if (exp == 0 && mantissa == 0) {
        exp = 0;
    } else if (exp == MASK(7)) {
        exp = 0xff;
    } else {
        exp += 0x40;
    }
    mantissa <<= 7;
    i = sgn << 31 | exp << 23 | mantissa;
    return i;
}

u32 f31tof32(u32 i) {
    i >>= 1;
    u32 sgn = (i >> 30) & 1;
    u32 exp = (i >> 23) & MASK(7);
    u32 mantissa = i & MASK(23);
    if (exp == 0 && mantissa == 0) {
        exp = 0;
    } else if (exp == MASK(7)) {
        exp = 0xff;
    } else {
        exp += 0x40;
    }
    i = sgn << 31 | exp << 23 | mantissa;
    return i;
}

FBInfo* fbcache_find(GPU* gpu, u32 color_paddr) {
    FBInfo* newfb = NULL;
    for (int i = 0; i < FB_MAX; i++) {
        if (gpu->fbs.d[i].color_paddr == color_paddr) {
            newfb = &gpu->fbs.d[i];
            break;
        }
    }
    if (newfb) LRU_use(gpu->fbs, newfb);
    return newfb;
}

FBInfo* fbcache_load(GPU* gpu, u32 color_paddr) {
    FBInfo* newfb = NULL;
    for (int i = 0; i < FB_MAX; i++) {
        if (gpu->fbs.d[i].color_paddr == color_paddr ||
            gpu->fbs.d[i].color_paddr == 0) {
            newfb = &gpu->fbs.d[i];
            break;
        }
    }
    if (!newfb) {
        newfb = LRU_eject(gpu->fbs);
        newfb->depth_paddr = 0;
        newfb->width = 0;
        newfb->height = 0;
    }
    newfb->color_paddr = color_paddr;
    LRU_use(gpu->fbs, newfb);
    return newfb;
}

void gpu_update_cur_fb(GPU* gpu) {
    if (gpu->cur_fb && gpu->cur_fb->color_paddr == gpu->io.fb.colorbuf_loc)
        return;

    gpu->cur_fb = fbcache_load(gpu, gpu->io.fb.colorbuf_loc << 3);
    gpu->cur_fb->depth_paddr = gpu->io.fb.depthbuf_loc << 3;

    linfo("drawing on fb at %x", gpu->cur_fb->color_paddr);

    glBindFramebuffer(GL_FRAMEBUFFER, gpu->cur_fb->fbo);

    u32 w = gpu->io.fb.dim.width;
    u32 h = gpu->io.fb.dim.height + 1;

    if (w != gpu->cur_fb->width || h != gpu->cur_fb->height) {
        gpu->cur_fb->width = w;
        gpu->cur_fb->height = h;

        glBindTexture(GL_TEXTURE_2D, gpu->cur_fb->color_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gpu->cur_fb->width,
                     gpu->cur_fb->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glBindTexture(GL_TEXTURE_2D, gpu->cur_fb->depth_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, gpu->cur_fb->width,
                     gpu->cur_fb->height, 0, GL_DEPTH_STENCIL,
                     GL_UNSIGNED_INT_24_8, NULL);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, gpu->cur_fb->color_tex, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                               GL_TEXTURE_2D, gpu->cur_fb->depth_tex, 0);

        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
}

void gpu_display_transfer(GPU* gpu, u32 paddr, bool top) {
    FBInfo* fb = fbcache_find(gpu, paddr);

    if (!fb) return;

    linfo("display transfer fb at %x to %s", paddr, top ? "top" : "bot");

    GLuint dst = top ? gpu->gl.textop : gpu->gl.texbot;

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fb->fbo);
    glBindTexture(GL_TEXTURE_2D, dst);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, fb->width, fb->height, 0);
}

void gpu_clear_fb(GPU* gpu, u32 paddr, u32 color) {
    for (int i = 0; i < FB_MAX; i++) {
        if (gpu->fbs.d[i].color_paddr == paddr) {
            LRU_use(gpu->fbs, &gpu->fbs.d[i]);
            glBindFramebuffer(GL_FRAMEBUFFER, gpu->fbs.d[i].fbo);
            glClearColor((color >> 24) / 256.f, ((color >> 16) & 0xff) / 256.f,
                         ((color >> 8) & 0xff) / 256.f, (color & 0xff) / 256.f);
            glClear(GL_COLOR_BUFFER_BIT);
            return;
        }
        if (gpu->fbs.d[i].depth_paddr == paddr) {
            LRU_use(gpu->fbs, &gpu->fbs.d[i]);
            glBindFramebuffer(GL_FRAMEBUFFER, gpu->fbs.d[i].fbo);
            glClear(GL_DEPTH_BUFFER_BIT);
            return;
        }
    }
}

TexInfo* texcache_load(GPU* gpu, u32 paddr) {
    TexInfo* tex = NULL;
    for (int i = 0; i < TEX_MAX; i++) {
        if (gpu->textures.d[i].paddr == paddr ||
            gpu->textures.d[i].paddr == 0) {
            tex = &gpu->textures.d[i];
            break;
        }
    }
    if (!tex) {
        tex = LRU_eject(gpu->textures);
        tex->width = 0;
        tex->height = 0;
    }
    tex->paddr = paddr;
    LRU_use(gpu->textures, tex);
    return tex;
}

u32 morton_swizzle(u32 w, u32 x, u32 y) {
    u32 swizzle[8] = {
        0x00, 0x01, 0x04, 0x05, 0x10, 0x11, 0x14, 0x15,
    };

    u32 tx = x >> 3;
    u32 fx = x & 7;
    u32 ty = y >> 3;
    u32 fy = y & 7;

    return (ty * (w >> 3) + tx) * 64 + (swizzle[fx] | swizzle[fy] << 1);
}

#define LOAD_TEX(t, glfmt, gltype)                                             \
    ({                                                                         \
        t* data = rawdata;                                                     \
                                                                               \
        t* pixels = malloc(sizeof(t) * tex->width * tex->height);              \
                                                                               \
        for (int x = 0; x < tex->width; x++) {                                 \
            for (int y = 0; y < tex->height; y++) {                            \
                pixels[y * tex->width + x] =                                   \
                    data[morton_swizzle(tex->width, x, y)];                    \
            }                                                                  \
        }                                                                      \
                                                                               \
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->width, tex->height, 0,    \
                     glfmt, gltype, pixels);                                   \
        free(pixels);                                                          \
        break;                                                                 \
    })

typedef struct {
    u8 d[3];
} u24;

void gpu_load_texture(GPU* gpu, int id, TexUnitRegs* regs, u32 fmt) {
    FBInfo* fb = fbcache_find(gpu, regs->addr << 3);
    glActiveTexture(GL_TEXTURE0 + id);
    if (fb) {
        glBindTexture(GL_TEXTURE_2D, fb->color_tex);
    } else {
        TexInfo* tex = texcache_load(gpu, regs->addr << 3);

        void* rawdata = PTR(tex->paddr);

        glBindTexture(GL_TEXTURE_2D, tex->tex);

        if (tex->width != regs->width || tex->height != regs->height ||
            tex->fmt != fmt) {
            tex->width = regs->width;
            tex->height = regs->height;
            tex->fmt = fmt;

            switch (fmt) {
                case 0:
                    LOAD_TEX(u32, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8);
                case 1:
                    LOAD_TEX(u24, GL_RGB, GL_UNSIGNED_BYTE);
                case 2:
                    LOAD_TEX(u16, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1);
                case 3:
                    LOAD_TEX(u16, GL_RGB, GL_UNSIGNED_SHORT_5_6_5);
                case 4:
                    LOAD_TEX(u16, GL_RGB, GL_UNSIGNED_SHORT_4_4_4_4);
                case 5:
                    LOAD_TEX(u8, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE);
                case 7:
                    LOAD_TEX(u8, GL_LUMINANCE, GL_UNSIGNED_BYTE);
                case 8:
                    LOAD_TEX(u8, GL_ALPHA, GL_UNSIGNED_BYTE);
                case 12: {
                    u8* dec = etc1_decompress_texture(rawdata, tex->width,
                                                      tex->height);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->width,
                                 tex->height, 0, GL_RGB, GL_UNSIGNED_BYTE, dec);
                    free(dec);
                    break;
                }
                default:
                    lwarn("unknown texture format %d", fmt);
            }
        }
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    texfilter[regs->param.min_filter]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    texfilter[regs->param.mag_filter]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    texwrap[regs->param.wrap_s & 3]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                    texwrap[regs->param.wrap_t & 3]);
}

void gpu_write_internalreg(GPU* gpu, u16 id, u32 param, u32 mask) {
    linfo("command %03x (0x%08x) & %08x (%f)", id, param, mask, I2F(param));
    gpu->io.w[id] &= ~mask;
    gpu->io.w[id] |= param & mask;
    switch (id) {
        case GPUREG(geom.drawarrays):
            gpu_update_gl_state(gpu);
            gpu_drawarrays(gpu);
            break;
        case GPUREG(geom.drawelements):
            gpu_update_gl_state(gpu);
            gpu_drawelements(gpu);
            break;
        case GPUREG(geom.fixattr_data[0])... GPUREG(geom.fixattr_data[2]): {
            fvec* fattr;
            if (gpu->io.geom.fixattr_idx == 0xf) {
                lwarn("immediate mode");
                break;
            } else {
                fattr = &gpu->fixattrs[gpu->io.geom.fixattr_idx];
            }
            switch (gpu->curfixi) {
                case 0: {
                    (*fattr)[3] = I2F(f24tof32(param >> 8));
                    gpu->curfixattr = (param & 0xff) << 16;
                    gpu->curfixi = 1;
                    break;
                }
                case 1: {
                    (*fattr)[2] = I2F(f24tof32(param >> 16 | gpu->curfixattr));
                    gpu->curfixattr = (param & MASK(16)) << 8;
                    gpu->curfixi = 2;
                    break;
                }
                case 2: {
                    (*fattr)[1] = I2F(f24tof32(param >> 24 | gpu->curfixattr));
                    (*fattr)[0] = I2F(f24tof32(param & MASK(24)));
                    gpu->curfixi = 0;
                    break;
                }
            }
            break;
        }
        case GPUREG(vsh.floatuniform_data[0])... GPUREG(
            vsh.floatuniform_data[7]): {
            fvec* uniform = &gpu->cst[gpu->io.vsh.floatuniform_idx];
            if (gpu->io.vsh.floatuniform_mode) {
                (*uniform)[3 - gpu->curunifi] = I2F(param);
                if (++gpu->curunifi == 4) {
                    gpu->curunifi = 0;
                    gpu->io.vsh.floatuniform_idx++;
                }
            } else {
                switch (gpu->curunifi) {
                    case 0: {
                        (*uniform)[3] = I2F(f24tof32(param >> 8));
                        gpu->curuniform = (param & 0xff) << 16;
                        gpu->curunifi = 1;
                        break;
                    }
                    case 1: {
                        (*uniform)[2] =
                            I2F(f24tof32(param >> 16 | gpu->curuniform));
                        gpu->curuniform = (param & MASK(16)) << 8;
                        gpu->curunifi = 2;
                        break;
                    }
                    case 2: {
                        (*uniform)[1] =
                            I2F(f24tof32(param >> 24 | gpu->curuniform));
                        (*uniform)[0] = I2F(f24tof32(param & MASK(24)));
                        gpu->curunifi = 0;
                        gpu->io.vsh.floatuniform_idx++;
                        break;
                    }
                }
            }
            break;
        }
        case GPUREG(vsh.codetrans_data[0])... GPUREG(vsh.codetrans_data[8]):
            gpu->progdata[gpu->io.vsh.codetrans_idx++] = param;
            break;
        case GPUREG(vsh.opdescs_data[0])... GPUREG(vsh.opdescs_data[8]):
            gpu->opdescs[gpu->io.vsh.opdescs_idx++] = param;
            break;
    }
}

void gpu_run_command_list(GPU* gpu, u32* cmds, u32 size) {
    gpu->cur_fb = NULL;

    u32* cur = cmds;
    u32* end = cmds + size;
    while (cur < end) {
        GPUCommand c = {cur[1]};
        u32 mask = 0;
        if (c.mask & BIT(0)) mask |= 0xff << 0;
        if (c.mask & BIT(1)) mask |= 0xff << 8;
        if (c.mask & BIT(2)) mask |= 0xff << 16;
        if (c.mask & BIT(3)) mask |= 0xff << 24;
        gpu_write_internalreg(gpu, c.id, cur[0], mask);
        cur += 2;
        if (c.incmode) c.id++;
        for (int i = 0; i < c.nparams; i++) {
            gpu_write_internalreg(gpu, c.id, *cur++, mask);
            if (c.incmode) c.id++;
        }
        if (c.nparams & 1) cur++;
    }
}

void load_vtx(GPU* gpu, int i) {
    memcpy(gpu->in, gpu->fixattrs, sizeof gpu->in);
    for (int vbo = 0; vbo < 12; vbo++) {
        void* vtx =
            PTR(gpu->io.geom.attr_base * 8 + gpu->io.geom.attrbuf[vbo].offset +
                i * gpu->io.geom.attrbuf[vbo].size);
        for (int c = 0; c < gpu->io.geom.attrbuf[vbo].count; c++) {
            int attr = (gpu->io.geom.attrbuf[vbo].comp >> 4 * c) & 0xf;
            if (attr >= 0xc) {
                vtx += 4 * (attr - 0xb);
                continue;
            }
            int type = (gpu->io.geom.attr_format >> 4 * attr) & 0xf;
            attr = (gpu->io.vsh.permutation >> 4 * attr) & 0xf;
            int size = (type >> 2) + 1;
            type &= 3;
            switch (type) {
                case 0: {
                    s8* ptr = vtx;
                    for (int k = 0; k < size; k++) {
                        gpu->in[attr][k] = *ptr++;
                    }
                    vtx = ptr;
                    break;
                }
                case 1: {
                    u8* ptr = vtx;
                    for (int k = 0; k < size; k++) {
                        gpu->in[attr][k] = *ptr++;
                    }
                    vtx = ptr;
                    break;
                }
                case 2: {
                    s16* ptr = vtx;
                    for (int k = 0; k < size; k++) {
                        gpu->in[attr][k] = *ptr++;
                    }
                    vtx = ptr;
                    break;
                }
                case 3: {
                    float* ptr = vtx;
                    for (int k = 0; k < size; k++) {
                        gpu->in[attr][k] = *ptr++;
                    }
                    vtx = ptr;
                    break;
                }
            }
            if (size < 2) gpu->in[attr][1] = 0.0f;
            if (size < 3) gpu->in[attr][2] = 0.0f;
            if (size < 4) gpu->in[attr][3] = 1.0f;
        }
    }
}

void store_vtx(GPU* gpu, int i, Vertex* vbuf) {
    for (int o = 0; o < 7; o++) {
        for (int j = 0; j < 4; j++) {
            u8 sem = gpu->io.raster.sh_outmap[o][j];
            if (sem < 0x18) vbuf[i].semantics[sem] = gpu->out[o][j];
        }
    }
}

void gpu_drawarrays(GPU* gpu) {
    linfo("drawing arrays nverts=%d primmode=%d", gpu->io.geom.nverts,
          gpu->io.geom.prim_config.mode);
    Vertex vbuf[gpu->io.geom.nverts];
    for (int i = 0; i < gpu->io.geom.nverts; i++) {
        load_vtx(gpu, i + gpu->io.geom.vtx_off);
        exec_vshader(gpu);
        store_vtx(gpu, i, vbuf);
    }
    glBufferData(GL_ARRAY_BUFFER, gpu->io.geom.nverts * sizeof(Vertex), vbuf,
                 GL_STREAM_DRAW);
    glDrawArrays(prim_mode[gpu->io.geom.prim_config.mode & 3], 0,
                 gpu->io.geom.nverts);
}

void gpu_drawelements(GPU* gpu) {
    linfo("drawing elements nverts=%d primmode=%d", gpu->io.geom.nverts,
          gpu->io.geom.prim_config.mode);

    u32 minind = 0xffff, maxind = 0;
    void* indexbuf = PTR(gpu->io.geom.attr_base * 8 + gpu->io.geom.indexbufoff);
    for (int i = 0; i < gpu->io.geom.nverts; i++) {
        int idx;
        if (gpu->io.geom.indexfmt) {
            idx = ((u16*) indexbuf)[i];
        } else {
            idx = ((u8*) indexbuf)[i];
        }
        if (idx < minind) minind = idx;
        if (idx > maxind) maxind = idx;
    }
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 gpu->io.geom.nverts * (gpu->io.geom.indexfmt ? 2 : 1),
                 indexbuf, GL_STREAM_DRAW);

    Vertex vbuf[maxind + 1 - minind];
    for (int i = minind; i <= maxind; i++) {
        load_vtx(gpu, i - minind);
        exec_vshader(gpu);
        store_vtx(gpu, i - minind, vbuf);
    }

    glBufferData(GL_ARRAY_BUFFER, (maxind + 1 - minind) * sizeof(Vertex), vbuf,
                 GL_STREAM_DRAW);
    glDrawElementsBaseVertex(
        prim_mode[gpu->io.geom.prim_config.mode & 3], gpu->io.geom.nverts,
        gpu->io.geom.indexfmt ? GL_UNSIGNED_SHORT : GL_UNSIGNED_BYTE, 0,
        -minind);
}

void gpu_update_gl_state(GPU* gpu) {
    switch (gpu->io.raster.facecull_config & 3) {
        case 0:
        case 3:
            glDisable(GL_CULL_FACE);
            break;
        case 1:
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
            break;
        case 2:
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            break;
    }

    glViewport(0, 0, 2 * I2F(f24tof32(gpu->io.raster.view_w)),
               2 * I2F(f24tof32(gpu->io.raster.view_h)));

    if (gpu->io.tex.texunit_cfg & 1) {
        glUniform1i(gpu->gl.uniforms.tex0enable, true);
        gpu_load_texture(gpu, 0, &gpu->io.tex.tex0, gpu->io.tex.tex0_fmt);
    } else {
        glUniform1i(gpu->gl.uniforms.tex0enable, false);
    }

    if (gpu->io.fb.color_mask.depthtest) {
        glDepthFunc(depth_func[gpu->io.fb.color_mask.depthfunc & 7]);
    } else {
        glDepthFunc(GL_ALWAYS);
    }
    glColorMask(gpu->io.fb.color_mask.red, gpu->io.fb.color_mask.green,
                gpu->io.fb.color_mask.blue, gpu->io.fb.color_mask.alpha);
    glDepthMask(gpu->io.fb.color_mask.depth);

    gpu_update_cur_fb(gpu);
}