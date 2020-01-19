/*
 Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies)
 Copyright (C) 2012 Igalia S.L.
 Copyright (C) 2012 Adobe Systems Incorporated

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Library General Public
 License as published by the Free Software Foundation; either
 version 2 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Library General Public License for more details.

 You should have received a copy of the GNU Library General Public License
 along with this library; see the file COPYING.LIB.  If not, write to
 the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 Boston, MA 02110-1301, USA.
 */

#include "config.h"
#include "TextureMapperGL.h"

#if USE(TEXTURE_MAPPER_GL)

#include "BitmapTextureGL.h"
#include "BitmapTexturePool.h"
#include "Extensions3D.h"
#include "FilterOperations.h"
#include "GraphicsContext.h"
#include "Image.h"
#include "LengthFunctions.h"
#include "NotImplemented.h"
#include "TextureMapperShaderProgram.h"
#include "Timer.h"
#include <wtf/HashMap.h>
#include <wtf/NeverDestroyed.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/TemporaryChange.h>

#if USE(CAIRO)
#include "CairoUtilities.h"
#include "RefPtrCairo.h"
#include <cairo.h>
#include <wtf/text/CString.h>
#endif

namespace WebCore {

class TextureMapperGLData {
    WTF_MAKE_FAST_ALLOCATED;
public:
    explicit TextureMapperGLData(GraphicsContext3D&);
    ~TextureMapperGLData();

    void initializeStencil();
    Platform3DObject getStaticVBO(GC3Denum target, GC3Dsizeiptr, const void* data);
    Ref<TextureMapperShaderProgram> getShaderProgram(TextureMapperShaderProgram::Options);

    TransformationMatrix projectionMatrix;
    TextureMapper::PaintFlags PaintFlags { 0 };
    GC3Dint previousProgram { 0 };
    GC3Dint targetFrameBuffer { 0 };
    bool didModifyStencil { false };
    GC3Dint previousScissorState { 0 };
    GC3Dint previousDepthState { 0 };
    GC3Dint viewport[4] { 0, };
    GC3Dint previousScissor[4] { 0, };
    RefPtr<BitmapTexture> currentSurface;
    const BitmapTextureGL::FilterInfo* filterInfo { nullptr };

private:
    class SharedGLData : public RefCounted<SharedGLData> {
    public:
        static Ref<SharedGLData> currentSharedGLData(GraphicsContext3D& context)
        {
            auto it = contextDataMap().find(context.platformGraphicsContext3D());
            if (it != contextDataMap().end())
                return *it->value;

            Ref<SharedGLData> data = adoptRef(*new SharedGLData(context));
            contextDataMap().add(context.platformGraphicsContext3D(), data.ptr());
            return data;
        }

        ~SharedGLData()
        {
            ASSERT(std::any_of(contextDataMap().begin(), contextDataMap().end(),
                [this](auto& entry) { return entry.value == this; }));
            contextDataMap().removeIf([this](auto& entry) { return entry.value == this; });
        }

    private:
        friend class TextureMapperGLData;

        using GLContextDataMap = HashMap<PlatformGraphicsContext3D, SharedGLData*>;
        static GLContextDataMap& contextDataMap()
        {
            static NeverDestroyed<GLContextDataMap> map;
            return map;
        }

        explicit SharedGLData(GraphicsContext3D& context)
        {
            contextDataMap().add(context.platformGraphicsContext3D(), this);
        }

        HashMap<TextureMapperShaderProgram::Options, RefPtr<TextureMapperShaderProgram>> m_programs;
    };

    GraphicsContext3D& m_context;
    Ref<SharedGLData> m_sharedGLData;
    HashMap<const void*, Platform3DObject> m_vbos;
};

TextureMapperGLData::TextureMapperGLData(GraphicsContext3D& context)
    : m_context(context)
    , m_sharedGLData(SharedGLData::currentSharedGLData(m_context))
{
}

TextureMapperGLData::~TextureMapperGLData()
{
    for (auto& entry : m_vbos)
        m_context.deleteBuffer(entry.value);
}

void TextureMapperGLData::initializeStencil()
{
    if (currentSurface) {
        static_cast<BitmapTextureGL*>(currentSurface.get())->initializeStencil();
        return;
    }

    if (didModifyStencil)
        return;

    m_context.clearStencil(0);
    m_context.clear(GraphicsContext3D::STENCIL_BUFFER_BIT);
    didModifyStencil = true;
}

Platform3DObject TextureMapperGLData::getStaticVBO(GC3Denum target, GC3Dsizeiptr size, const void* data)
{
    auto addResult = m_vbos.ensure(data,
        [this, target, size, data] {
            Platform3DObject vbo = m_context.createBuffer();
            m_context.bindBuffer(target, vbo);
            m_context.bufferData(target, size, data, GraphicsContext3D::STATIC_DRAW);
            return vbo;
        });
    return addResult.iterator->value;
}

Ref<TextureMapperShaderProgram> TextureMapperGLData::getShaderProgram(TextureMapperShaderProgram::Options options)
{
    auto addResult = m_sharedGLData->m_programs.ensure(options,
        [this, options] { return TextureMapperShaderProgram::create(Ref<GraphicsContext3D>(m_context), options); });
    return *addResult.iterator->value;
}

TextureMapperGL::TextureMapperGL()
    : m_enableEdgeDistanceAntialiasing(false)
{
    m_context3D = GraphicsContext3D::createForCurrentGLContext();
    ASSERT(m_context3D);

    m_data = new TextureMapperGLData(*m_context3D);
#if USE(TEXTURE_MAPPER_GL)
    m_texturePool = std::make_unique<BitmapTexturePool>(m_context3D.copyRef());
#endif
}

ClipStack& TextureMapperGL::clipStack()
{
    return data().currentSurface ? toBitmapTextureGL(data().currentSurface.get())->clipStack() : m_clipStack;
}

void TextureMapperGL::beginPainting(PaintFlags flags)
{
    m_context3D->getIntegerv(GraphicsContext3D::CURRENT_PROGRAM, &data().previousProgram);
    data().previousScissorState = m_context3D->isEnabled(GraphicsContext3D::SCISSOR_TEST);
    data().previousDepthState = m_context3D->isEnabled(GraphicsContext3D::DEPTH_TEST);
    m_context3D->disable(GraphicsContext3D::DEPTH_TEST);
    m_context3D->enable(GraphicsContext3D::SCISSOR_TEST);
    data().didModifyStencil = false;
    m_context3D->depthMask(0);
    m_context3D->getIntegerv(GraphicsContext3D::VIEWPORT, data().viewport);
    m_context3D->getIntegerv(GraphicsContext3D::SCISSOR_BOX, data().previousScissor);
    m_clipStack.reset(IntRect(0, 0, data().viewport[2], data().viewport[3]), flags & PaintingMirrored ? ClipStack::YAxisMode::Default : ClipStack::YAxisMode::Inverted);
    m_context3D->getIntegerv(GraphicsContext3D::FRAMEBUFFER_BINDING, &data().targetFrameBuffer);
    data().PaintFlags = flags;
    bindSurface(0);
}

void TextureMapperGL::endPainting()
{
    if (data().didModifyStencil) {
        m_context3D->clearStencil(1);
        m_context3D->clear(GraphicsContext3D::STENCIL_BUFFER_BIT);
    }

    m_context3D->useProgram(data().previousProgram);

    m_context3D->scissor(data().previousScissor[0], data().previousScissor[1], data().previousScissor[2], data().previousScissor[3]);
    if (data().previousScissorState)
        m_context3D->enable(GraphicsContext3D::SCISSOR_TEST);
    else
        m_context3D->disable(GraphicsContext3D::SCISSOR_TEST);

    if (data().previousDepthState)
        m_context3D->enable(GraphicsContext3D::DEPTH_TEST);
    else
        m_context3D->disable(GraphicsContext3D::DEPTH_TEST);
}

void TextureMapperGL::drawBorder(const Color& color, float width, const FloatRect& targetRect, const TransformationMatrix& modelViewMatrix)
{
    if (clipStack().isCurrentScissorBoxEmpty())
        return;

    Ref<TextureMapperShaderProgram> program = data().getShaderProgram(TextureMapperShaderProgram::SolidColor);
    m_context3D->useProgram(program->programID());

    float r, g, b, a;
    Color(premultipliedARGBFromColor(color)).getRGBA(r, g, b, a);
    m_context3D->uniform4f(program->colorLocation(), r, g, b, a);
    m_context3D->lineWidth(width);

    draw(targetRect, modelViewMatrix, program.get(), GraphicsContext3D::LINE_LOOP, color.hasAlpha() ? ShouldBlend : 0);
}

// FIXME: drawNumber() should save a number texture-atlas and re-use whenever possible.
void TextureMapperGL::drawNumber(int number, const Color& color, const FloatPoint& targetPoint, const TransformationMatrix& modelViewMatrix)
{
    int pointSize = 8;

#if USE(CAIRO)
    CString counterString = String::number(number).ascii();
    // cairo_text_extents() requires a cairo_t, so dimensions need to be guesstimated.
    int width = counterString.length() * pointSize * 1.2;
    int height = pointSize * 1.5;

    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t* cr = cairo_create(surface);

    float r, g, b, a;
    color.getRGBA(r, g, b, a);
    cairo_set_source_rgba(cr, b, g, r, a); // Since we won't swap R+B when uploading a texture, paint with the swapped R+B color.
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_fill(cr);

    cairo_select_font_face(cr, "Monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, pointSize);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_move_to(cr, 2, pointSize);
    cairo_show_text(cr, counterString.data());

    IntSize size(width, height);
    IntRect sourceRect(IntPoint::zero(), size);
    IntRect targetRect(roundedIntPoint(targetPoint), size);

    RefPtr<BitmapTexture> texture = acquireTextureFromPool(size);
    const unsigned char* bits = cairo_image_surface_get_data(surface);
    int stride = cairo_image_surface_get_stride(surface);
    static_cast<BitmapTextureGL*>(texture.get())->updateContentsNoSwizzle(bits, sourceRect, IntPoint::zero(), stride);
    drawTexture(*texture, targetRect, modelViewMatrix, 1.0f, AllEdges);

    cairo_surface_destroy(surface);
    cairo_destroy(cr);

#else
    UNUSED_PARAM(number);
    UNUSED_PARAM(pointSize);
    UNUSED_PARAM(targetPoint);
    UNUSED_PARAM(modelViewMatrix);
    notImplemented();
#endif
}

static TextureMapperShaderProgram::Options optionsForFilterType(FilterOperation::OperationType type, unsigned pass)
{
    switch (type) {
    case FilterOperation::GRAYSCALE:
        return TextureMapperShaderProgram::Texture | TextureMapperShaderProgram::GrayscaleFilter;
    case FilterOperation::SEPIA:
        return TextureMapperShaderProgram::Texture | TextureMapperShaderProgram::SepiaFilter;
    case FilterOperation::SATURATE:
        return TextureMapperShaderProgram::Texture | TextureMapperShaderProgram::SaturateFilter;
    case FilterOperation::HUE_ROTATE:
        return TextureMapperShaderProgram::Texture | TextureMapperShaderProgram::HueRotateFilter;
    case FilterOperation::INVERT:
        return TextureMapperShaderProgram::Texture | TextureMapperShaderProgram::InvertFilter;
    case FilterOperation::BRIGHTNESS:
        return TextureMapperShaderProgram::Texture | TextureMapperShaderProgram::BrightnessFilter;
    case FilterOperation::CONTRAST:
        return TextureMapperShaderProgram::Texture | TextureMapperShaderProgram::ContrastFilter;
    case FilterOperation::OPACITY:
        return TextureMapperShaderProgram::Texture | TextureMapperShaderProgram::OpacityFilter;
    case FilterOperation::BLUR:
        return TextureMapperShaderProgram::BlurFilter;
    case FilterOperation::DROP_SHADOW:
        return TextureMapperShaderProgram::AlphaBlur
            | (pass ? TextureMapperShaderProgram::ContentTexture | TextureMapperShaderProgram::SolidColor: 0);
    default:
        ASSERT_NOT_REACHED();
        return 0;
    }
}

// Create a normal distribution of 21 values between -2 and 2.
static const unsigned GaussianKernelHalfWidth = 11;
static const float GaussianKernelStep = 0.2;

static inline float gauss(float x)
{
    return exp(-(x * x) / 2.);
}

static float* gaussianKernel()
{
    static bool prepared = false;
    static float kernel[GaussianKernelHalfWidth] = {0, };

    if (prepared)
        return kernel;

    kernel[0] = gauss(0);
    float sum = kernel[0];
    for (unsigned i = 1; i < GaussianKernelHalfWidth; ++i) {
        kernel[i] = gauss(i * GaussianKernelStep);
        sum += 2 * kernel[i];
    }

    // Normalize the kernel.
    float scale = 1 / sum;
    for (unsigned i = 0; i < GaussianKernelHalfWidth; ++i)
        kernel[i] *= scale;

    prepared = true;
    return kernel;
}

static void prepareFilterProgram(TextureMapperShaderProgram& program, const FilterOperation& operation, unsigned pass, const IntSize& size, GC3Duint contentTexture)
{
    Ref<GraphicsContext3D> context = program.context();
    context->useProgram(program.programID());

    switch (operation.type()) {
    case FilterOperation::GRAYSCALE:
    case FilterOperation::SEPIA:
    case FilterOperation::SATURATE:
    case FilterOperation::HUE_ROTATE:
        context->uniform1f(program.filterAmountLocation(), static_cast<const BasicColorMatrixFilterOperation&>(operation).amount());
        break;
    case FilterOperation::INVERT:
    case FilterOperation::BRIGHTNESS:
    case FilterOperation::CONTRAST:
    case FilterOperation::OPACITY:
        context->uniform1f(program.filterAmountLocation(), static_cast<const BasicComponentTransferFilterOperation&>(operation).amount());
        break;
    case FilterOperation::BLUR: {
        const BlurFilterOperation& blur = static_cast<const BlurFilterOperation&>(operation);
        FloatSize radius;

        // Blur is done in two passes, first horizontally and then vertically. The same shader is used for both.
        if (pass)
            radius.setHeight(floatValueForLength(blur.stdDeviation(), size.height()) / size.height());
        else
            radius.setWidth(floatValueForLength(blur.stdDeviation(), size.width()) / size.width());

        context->uniform2f(program.blurRadiusLocation(), radius.width(), radius.height());
        context->uniform1fv(program.gaussianKernelLocation(), GaussianKernelHalfWidth, gaussianKernel());
        break;
    }
    case FilterOperation::DROP_SHADOW: {
        const DropShadowFilterOperation& shadow = static_cast<const DropShadowFilterOperation&>(operation);
        context->uniform1fv(program.gaussianKernelLocation(), GaussianKernelHalfWidth, gaussianKernel());
        switch (pass) {
        case 0:
            // First pass: horizontal alpha blur.
            context->uniform2f(program.blurRadiusLocation(), shadow.stdDeviation() / float(size.width()), 0);
            context->uniform2f(program.shadowOffsetLocation(), float(shadow.location().x()) / float(size.width()), float(shadow.location().y()) / float(size.height()));
            break;
        case 1:
            // Second pass: we need the shadow color and the content texture for compositing.
            float r, g, b, a;
            Color(premultipliedARGBFromColor(shadow.color())).getRGBA(r, g, b, a);
            context->uniform4f(program.colorLocation(), r, g, b, a);
            context->uniform2f(program.blurRadiusLocation(), 0, shadow.stdDeviation() / float(size.height()));
            context->uniform2f(program.shadowOffsetLocation(), 0, 0);
            context->activeTexture(GraphicsContext3D::TEXTURE1);
            context->bindTexture(GraphicsContext3D::TEXTURE_2D, contentTexture);
            context->uniform1i(program.contentTextureLocation(), 1);
            break;
        }
        break;
    }
    default:
        break;
    }
}

void TextureMapperGL::drawTexture(const BitmapTexture& texture, const FloatRect& targetRect, const TransformationMatrix& matrix, float opacity, unsigned exposedEdges)
{
    if (!texture.isValid())
        return;

    if (clipStack().isCurrentScissorBoxEmpty())
        return;

    const BitmapTextureGL& textureGL = static_cast<const BitmapTextureGL&>(texture);
    TemporaryChange<const BitmapTextureGL::FilterInfo*> filterInfo(data().filterInfo, textureGL.filterInfo());

    drawTexture(textureGL.id(), textureGL.isOpaque() ? 0 : ShouldBlend, textureGL.size(), targetRect, matrix, opacity, exposedEdges);
}

static bool driverSupportsNPOTTextures(GraphicsContext3D& context)
{
    if (context.isGLES2Compliant()) {
        static bool supportsNPOTTextures = context.getExtensions()->supports("GL_OES_texture_npot");
        return supportsNPOTTextures;
    }

    return true;
}

void TextureMapperGL::drawTexture(Platform3DObject texture, Flags flags, const IntSize& textureSize, const FloatRect& targetRect, const TransformationMatrix& modelViewMatrix, float opacity, unsigned exposedEdges)
{
    bool useRect = flags & ShouldUseARBTextureRect;
    bool useAntialiasing = m_enableEdgeDistanceAntialiasing
        && exposedEdges == AllEdges
        && !modelViewMatrix.mapQuad(targetRect).isRectilinear();

    TextureMapperShaderProgram::Options options = TextureMapperShaderProgram::Texture;
    if (useRect)
        options |= TextureMapperShaderProgram::Rect;
    if (opacity < 1)
        options |= TextureMapperShaderProgram::Opacity;
    if (useAntialiasing) {
        options |= TextureMapperShaderProgram::Antialiasing;
        flags |= ShouldAntialias;
    }
    if (wrapMode() == RepeatWrap && !driverSupportsNPOTTextures(*m_context3D))
        options |= TextureMapperShaderProgram::ManualRepeat;

    RefPtr<FilterOperation> filter = data().filterInfo ? data().filterInfo->filter: 0;
    GC3Duint filterContentTextureID = 0;

    if (filter) {
        if (data().filterInfo->contentTexture)
            filterContentTextureID = toBitmapTextureGL(data().filterInfo->contentTexture.get())->id();
        options |= optionsForFilterType(filter->type(), data().filterInfo->pass);
        if (filter->affectsOpacity())
            flags |= ShouldBlend;
    }

    if (useAntialiasing || opacity < 1)
        flags |= ShouldBlend;

    Ref<TextureMapperShaderProgram> program = data().getShaderProgram(options);

    if (filter)
        prepareFilterProgram(program.get(), *filter.get(), data().filterInfo->pass, textureSize, filterContentTextureID);

    drawTexturedQuadWithProgram(program.get(), texture, flags, textureSize, targetRect, modelViewMatrix, opacity);
}

void TextureMapperGL::drawSolidColor(const FloatRect& rect, const TransformationMatrix& matrix, const Color& color)
{
    Flags flags = 0;
    TextureMapperShaderProgram::Options options = TextureMapperShaderProgram::SolidColor;
    if (!matrix.mapQuad(rect).isRectilinear()) {
        options |= TextureMapperShaderProgram::Antialiasing;
        flags |= ShouldBlend | ShouldAntialias;
    }

    Ref<TextureMapperShaderProgram> program = data().getShaderProgram(options);
    m_context3D->useProgram(program->programID());

    float r, g, b, a;
    Color(premultipliedARGBFromColor(color)).getRGBA(r, g, b, a);
    m_context3D->uniform4f(program->colorLocation(), r, g, b, a);
    if (a < 1)
        flags |= ShouldBlend;

    draw(rect, matrix, program.get(), GraphicsContext3D::TRIANGLE_FAN, flags);
}

void TextureMapperGL::drawEdgeTriangles(TextureMapperShaderProgram& program)
{
    const GC3Dfloat left = 0;
    const GC3Dfloat top = 0;
    const GC3Dfloat right = 1;
    const GC3Dfloat bottom = 1;
    const GC3Dfloat center = 0.5;

// Each 4d triangle consists of a center point and two edge points, where the zw coordinates
// of each vertex equals the nearest point to the vertex on the edge.
#define SIDE_TRIANGLE_DATA(x1, y1, x2, y2) \
    x1, y1, x1, y1, \
    x2, y2, x2, y2, \
    center, center, (x1 + x2) / 2, (y1 + y2) / 2

    static const GC3Dfloat unitRectSideTriangles[] = {
        SIDE_TRIANGLE_DATA(left, top, right, top),
        SIDE_TRIANGLE_DATA(left, top, left, bottom),
        SIDE_TRIANGLE_DATA(right, top, right, bottom),
        SIDE_TRIANGLE_DATA(left, bottom, right, bottom)
    };
#undef SIDE_TRIANGLE_DATA

    Platform3DObject vbo = data().getStaticVBO(GraphicsContext3D::ARRAY_BUFFER, sizeof(GC3Dfloat) * 48, unitRectSideTriangles);
    m_context3D->bindBuffer(GraphicsContext3D::ARRAY_BUFFER, vbo);
    m_context3D->vertexAttribPointer(program.vertexLocation(), 4, GraphicsContext3D::FLOAT, false, 0, 0);
    m_context3D->drawArrays(GraphicsContext3D::TRIANGLES, 0, 12);
    m_context3D->bindBuffer(GraphicsContext3D::ARRAY_BUFFER, 0);
}

void TextureMapperGL::drawUnitRect(TextureMapperShaderProgram& program, GC3Denum drawingMode)
{
    static const GC3Dfloat unitRect[] = { 0, 0, 1, 0, 1, 1, 0, 1 };
    Platform3DObject vbo = data().getStaticVBO(GraphicsContext3D::ARRAY_BUFFER, sizeof(GC3Dfloat) * 8, unitRect);
    m_context3D->bindBuffer(GraphicsContext3D::ARRAY_BUFFER, vbo);
    m_context3D->vertexAttribPointer(program.vertexLocation(), 2, GraphicsContext3D::FLOAT, false, 0, 0);
    m_context3D->drawArrays(drawingMode, 0, 4);
    m_context3D->bindBuffer(GraphicsContext3D::ARRAY_BUFFER, 0);
}

void TextureMapperGL::draw(const FloatRect& rect, const TransformationMatrix& modelViewMatrix, TextureMapperShaderProgram& program, GC3Denum drawingMode, Flags flags)
{
    TransformationMatrix matrix(modelViewMatrix);
    matrix.multiply(TransformationMatrix::rectToRect(FloatRect(0, 0, 1, 1), rect));

    m_context3D->enableVertexAttribArray(program.vertexLocation());
    program.setMatrix(program.modelViewMatrixLocation(), matrix);
    program.setMatrix(program.projectionMatrixLocation(), data().projectionMatrix);

    if (isInMaskMode()) {
        m_context3D->blendFunc(GraphicsContext3D::ZERO, GraphicsContext3D::SRC_ALPHA);
        m_context3D->enable(GraphicsContext3D::BLEND);
    } else {
        if (flags & ShouldBlend) {
            m_context3D->blendFunc(GraphicsContext3D::ONE, GraphicsContext3D::ONE_MINUS_SRC_ALPHA);
            m_context3D->enable(GraphicsContext3D::BLEND);
        } else
            m_context3D->disable(GraphicsContext3D::BLEND);
    }

    if (flags & ShouldAntialias)
        drawEdgeTriangles(program);
    else
        drawUnitRect(program, drawingMode);

    m_context3D->disableVertexAttribArray(program.vertexLocation());
    m_context3D->blendFunc(GraphicsContext3D::ONE, GraphicsContext3D::ONE_MINUS_SRC_ALPHA);
    m_context3D->enable(GraphicsContext3D::BLEND);
}

void TextureMapperGL::drawTexturedQuadWithProgram(TextureMapperShaderProgram& program, uint32_t texture, Flags flags, const IntSize& size, const FloatRect& rect, const TransformationMatrix& modelViewMatrix, float opacity)
{
    m_context3D->useProgram(program.programID());
    m_context3D->activeTexture(GraphicsContext3D::TEXTURE0);
    GC3Denum target = flags & ShouldUseARBTextureRect ? GC3Denum(Extensions3D::TEXTURE_RECTANGLE_ARB) : GC3Denum(GraphicsContext3D::TEXTURE_2D);
    m_context3D->bindTexture(target, texture);
    m_context3D->uniform1i(program.samplerLocation(), 0);
    if (wrapMode() == RepeatWrap && driverSupportsNPOTTextures(*m_context3D)) {
        m_context3D->texParameteri(GraphicsContext3D::TEXTURE_2D, GraphicsContext3D::TEXTURE_WRAP_S, GraphicsContext3D::REPEAT);
        m_context3D->texParameteri(GraphicsContext3D::TEXTURE_2D, GraphicsContext3D::TEXTURE_WRAP_T, GraphicsContext3D::REPEAT);
    }

    TransformationMatrix patternTransform = this->patternTransform();
    if (flags & ShouldRotateTexture90) {
        patternTransform.rotate(-90);
        patternTransform.translate(-1, 0);
    }
    if (flags & ShouldRotateTexture180) {
        patternTransform.rotate(180);
        patternTransform.translate(-1, -1);
    }
    if (flags & ShouldRotateTexture270) {
        patternTransform.rotate(-270);
        patternTransform.translate(0, -1);
    }
    if (flags & ShouldFlipTexture)
        patternTransform.flipY();
    if (flags & ShouldUseARBTextureRect)
        patternTransform.scaleNonUniform(size.width(), size.height());
    if (flags & ShouldFlipTexture)
        patternTransform.translate(0, -1);

    program.setMatrix(program.textureSpaceMatrixLocation(), patternTransform);
    m_context3D->uniform1f(program.opacityLocation(), opacity);

    if (opacity < 1)
        flags |= ShouldBlend;

    draw(rect, modelViewMatrix, program, GraphicsContext3D::TRIANGLE_FAN, flags);
    m_context3D->texParameteri(GraphicsContext3D::TEXTURE_2D, GraphicsContext3D::TEXTURE_WRAP_S, GraphicsContext3D::CLAMP_TO_EDGE);
    m_context3D->texParameteri(GraphicsContext3D::TEXTURE_2D, GraphicsContext3D::TEXTURE_WRAP_T, GraphicsContext3D::CLAMP_TO_EDGE);
}

void TextureMapperGL::drawFiltered(const BitmapTexture& sampler, const BitmapTexture* contentTexture, const FilterOperation& filter, int pass)
{
    // For standard filters, we always draw the whole texture without transformations.
    TextureMapperShaderProgram::Options options = optionsForFilterType(filter.type(), pass);
    Ref<TextureMapperShaderProgram> program = data().getShaderProgram(options);

    prepareFilterProgram(program.get(), filter, pass, sampler.contentSize(), contentTexture ? static_cast<const BitmapTextureGL*>(contentTexture)->id() : 0);
    FloatRect targetRect(IntPoint::zero(), sampler.contentSize());
    drawTexturedQuadWithProgram(program.get(), static_cast<const BitmapTextureGL&>(sampler).id(), 0, IntSize(1, 1), targetRect, TransformationMatrix(), 1);
}

static inline TransformationMatrix createProjectionMatrix(const IntSize& size, bool mirrored)
{
    const float nearValue = 9999999;
    const float farValue = -99999;

    return TransformationMatrix(2.0 / float(size.width()), 0, 0, 0,
                                0, (mirrored ? 2.0 : -2.0) / float(size.height()), 0, 0,
                                0, 0, -2.f / (farValue - nearValue), 0,
                                -1, mirrored ? -1 : 1, -(farValue + nearValue) / (farValue - nearValue), 1);
}

TextureMapperGL::~TextureMapperGL()
{
    delete m_data;
}

void TextureMapperGL::bindDefaultSurface()
{
    m_context3D->bindFramebuffer(GraphicsContext3D::FRAMEBUFFER, data().targetFrameBuffer);
    auto& viewport = data().viewport;
    data().projectionMatrix = createProjectionMatrix(IntSize(viewport[2], viewport[3]), data().PaintFlags & PaintingMirrored);
    m_context3D->viewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    m_clipStack.apply(*m_context3D);
    data().currentSurface = nullptr;
}

void TextureMapperGL::bindSurface(BitmapTexture *surface)
{
    if (!surface) {
        bindDefaultSurface();
        return;
    }

    static_cast<BitmapTextureGL*>(surface)->bindAsSurface(m_context3D.get());
    data().projectionMatrix = createProjectionMatrix(surface->size(), true /* mirrored */);
    data().currentSurface = surface;
}

BitmapTexture* TextureMapperGL::currentSurface()
{
    return data().currentSurface.get();
}

bool TextureMapperGL::beginScissorClip(const TransformationMatrix& modelViewMatrix, const FloatRect& targetRect)
{
    // 3D transforms are currently not supported in scissor clipping
    // resulting in cropped surfaces when z>0.
    if (!modelViewMatrix.isAffine())
        return false;

    FloatQuad quad = modelViewMatrix.projectQuad(targetRect);
    IntRect rect = quad.enclosingBoundingBox();

    // Only use scissors on rectilinear clips.
    if (!quad.isRectilinear() || rect.isEmpty())
        return false;

    clipStack().intersect(rect);
    clipStack().applyIfNeeded(*m_context3D);
    return true;
}

void TextureMapperGL::beginClip(const TransformationMatrix& modelViewMatrix, const FloatRect& targetRect)
{
    clipStack().push();
    if (beginScissorClip(modelViewMatrix, targetRect))
        return;

    data().initializeStencil();

    Ref<TextureMapperShaderProgram> program = data().getShaderProgram(TextureMapperShaderProgram::SolidColor);

    m_context3D->useProgram(program->programID());
    m_context3D->enableVertexAttribArray(program->vertexLocation());
    const GC3Dfloat unitRect[] = {0, 0, 1, 0, 1, 1, 0, 1};
    Platform3DObject vbo = data().getStaticVBO(GraphicsContext3D::ARRAY_BUFFER, sizeof(GC3Dfloat) * 8, unitRect);
    m_context3D->bindBuffer(GraphicsContext3D::ARRAY_BUFFER, vbo);
    m_context3D->vertexAttribPointer(program->vertexLocation(), 2, GraphicsContext3D::FLOAT, false, 0, 0);

    TransformationMatrix matrix(modelViewMatrix);
    matrix.multiply(TransformationMatrix::rectToRect(FloatRect(0, 0, 1, 1), targetRect));

    static const TransformationMatrix fullProjectionMatrix = TransformationMatrix::rectToRect(FloatRect(0, 0, 1, 1), FloatRect(-1, -1, 2, 2));

    int stencilIndex = clipStack().getStencilIndex();

    m_context3D->enable(GraphicsContext3D::STENCIL_TEST);

    // Make sure we don't do any actual drawing.
    m_context3D->stencilFunc(GraphicsContext3D::NEVER, stencilIndex, stencilIndex);

    // Operate only on the stencilIndex and above.
    m_context3D->stencilMask(0xff & ~(stencilIndex - 1));

    // First clear the entire buffer at the current index.
    program->setMatrix(program->projectionMatrixLocation(), fullProjectionMatrix);
    program->setMatrix(program->modelViewMatrixLocation(), TransformationMatrix());
    m_context3D->stencilOp(GraphicsContext3D::ZERO, GraphicsContext3D::ZERO, GraphicsContext3D::ZERO);
    m_context3D->drawArrays(GraphicsContext3D::TRIANGLE_FAN, 0, 4);

    // Now apply the current index to the new quad.
    m_context3D->stencilOp(GraphicsContext3D::REPLACE, GraphicsContext3D::REPLACE, GraphicsContext3D::REPLACE);
    program->setMatrix(program->projectionMatrixLocation(), data().projectionMatrix);
    program->setMatrix(program->modelViewMatrixLocation(), matrix);
    m_context3D->drawArrays(GraphicsContext3D::TRIANGLE_FAN, 0, 4);

    // Clear the state.
    m_context3D->bindBuffer(GraphicsContext3D::ARRAY_BUFFER, 0);
    m_context3D->disableVertexAttribArray(program->vertexLocation());
    m_context3D->stencilMask(0);

    // Increase stencilIndex and apply stencil testing.
    clipStack().setStencilIndex(stencilIndex * 2);
    clipStack().applyIfNeeded(*m_context3D);
}

void TextureMapperGL::endClip()
{
    clipStack().pop();
    clipStack().applyIfNeeded(*m_context3D);
}

IntRect TextureMapperGL::clipBounds()
{
    return clipStack().current().scissorBox;
}

PassRefPtr<BitmapTexture> TextureMapperGL::createTexture()
{
    BitmapTextureGL* texture = new BitmapTextureGL(m_context3D);
    return adoptRef(texture);
}

std::unique_ptr<TextureMapper> TextureMapper::platformCreateAccelerated()
{
    return std::make_unique<TextureMapperGL>();
}

};

#endif // USE(TEXTURE_MAPPER_GL)
