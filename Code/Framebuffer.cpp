#include "Framebuffer.h"

#include <glad/glad.h>

static void CreateTextures(uint32_t* data)
{
	glGenTextures(GL_TEXTURE_2D, data);
}

static void AttachColorTexture(uint32_t id, GLenum internalFormat, GLenum format, GLenum type, uint32_t width, uint32_t height, int index)
{
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_2D, id, 0);
}

Framebuffer::Framebuffer(FramebufferData spec) : specification(spec)
{
	for (int i = 0; i < spec.spec.attachments.size(); ++i)
	{
		if (spec.spec.attachments[i] == TextureAttachmentType::DEPTH)
			depth = spec.spec.attachments[i];
		else
			colorTextures.emplace_back(spec.spec.attachments[i]);
	}

	Init();
}

Framebuffer::~Framebuffer()
{
}

void Framebuffer::Init()
{
	glGenFramebuffers(1, &framebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferId);

	colorAttachments.resize(colorTextures.size());
	for (int i = 0; i < colorAttachments.size(); ++i)
	{
		CreateTextures(&colorAttachments[i]);
		glBindTexture(GL_TEXTURE_2D, colorAttachments[i]);

		switch (colorTextures[i])
		{
		case TextureAttachmentType::RGBA8:
			AttachColorTexture(colorAttachments[i], GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, specification.width, specification.height, i);
			break;
		case TextureAttachmentType::RGBA16:
			AttachColorTexture(colorAttachments[i], GL_RGBA16F, GL_RGBA, GL_FLOAT, specification.width, specification.height, i);
			break;
		}
	}

	if (depth != TextureAttachmentType::DEPTH)
	{
		CreateTextures(&depthAttachment);
		glBindTexture(GL_TEXTURE_2D, depthAttachment);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, specification.width, specification.height);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthAttachment, 0);
	}

	if (colorAttachments.size() > 1)
	{
		GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		glDrawBuffers(colorAttachments.size(), buffers);
	}
	else if (colorAttachments.empty())
	{
		// Only depth pass
		glDrawBuffer(GL_NONE);
	}

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
	{
		bool ret = true;
		ret = false;
	}
}

void Framebuffer::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferId);
}

void Framebuffer::Unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}