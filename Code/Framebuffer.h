#pragma once

#include "platform.h"
#include <vector>

enum class TextureAttachmentType
{
	NONE = 0,

	RGBA8 = 1,
	RGBA16 = 2,
	DEPTH = 3
};

struct FramebufferSpecification
{
	FramebufferSpecification(std::initializer_list<TextureAttachmentType> attach) 
		: attachments(attach) {}

	std::vector<TextureAttachmentType> attachments;
};

struct FramebufferData
{
	u32 width = 1280;
	u32 height = 720;

	FramebufferSpecification spec;
};

class Framebuffer
{
public:
	Framebuffer(FramebufferData spec);
	~Framebuffer();

	void Init();

	void Bind();
	void Unbind();

private:
	FramebufferData specification;

	std::vector<TextureAttachmentType> colorTextures;
	TextureAttachmentType depth;

	std::vector<u32> colorAttachments;
	u32 depthAttachment;

	u32 framebufferId;
};