#pragma once

#include "platform.h"
#include <vector>

enum class FramebufferTextureFormat
{
	NONE = 0,

	// Color
	RGBA8 = 1,
	RGBA16 = 2,

	// Red channel
	RED_INTEGER = 3,

	// Depth / Stencil
	DEPTH24_STENCIL8 = 4
};

struct FramebufferTextureSpecification
{
	FramebufferTextureSpecification() = default;
	FramebufferTextureSpecification(FramebufferTextureFormat format)
		: textureFormat(format) {}

	FramebufferTextureFormat textureFormat = FramebufferTextureFormat::NONE;
};

struct FramebufferAttachmentSpecification
{
	FramebufferAttachmentSpecification() = default;
	FramebufferAttachmentSpecification(std::initializer_list<FramebufferTextureSpecification> attach)
		: attachments(attach) {}

	std::vector<FramebufferTextureSpecification> attachments;
};

struct FramebufferSpecification
{
	uint32_t width = 1280;
	uint32_t height = 720;

	FramebufferAttachmentSpecification attachments;
};

class Framebuffer
{
public:
	Framebuffer(u32 numColorAttachments, int w, int h);
	~Framebuffer();

	void Init(u32 numColorAttachments);

	void Bind();
	void Unbind();

	void BindTextures();

	u32 GetColorAttachment(u32 slot = 0) { return colorAttachments[slot]; }

private:
	u32 framebufferID;

	std::vector<u32> colorAttachments;
	u32 depthAttachment;

	int width;
	int height;
};