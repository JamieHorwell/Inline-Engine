#pragma once

#include "../GraphicsNode.hpp"
#include "../ResourceView.hpp"
#include "../PipelineTypes.hpp"

#include <cmath>


namespace inl::gxeng::nodes {



/// <summary>
/// Creates a texture with the given parameters.
/// Inputs: width, height, format, texture array element count, usage, iscubemap.
/// Output: a new texture.
/// </summary>
/// <remarks>
/// A general philosophy of the pipeline is that each node that renders to texture
/// should take the render target(s) as its input.
/// This node's sole pupose is to create render target textures for the pipeline.
/// </remarks>
class CreateTexture :
	virtual public GraphicsNode,
	public GraphicsTask,
	public InputPortConfig<unsigned, unsigned, gxapi::eFormat, uint16_t, TextureUsage, bool, bool>,
	public OutputPortConfig<gxeng::Texture2D>
{
public:
	static const char* Info_GetName() { return "CreateTexture"; }
	virtual void Update() override {}

	static int getNumMips(int w, int h, int d)
	{
		return 1 + std::floor(std::log2(std::max(std::max(w, h), d)));
	}

	virtual void Notify(InputPortBase* sender) override {}

	virtual void Initialize(EngineContext& context) override {
		GraphicsNode::SetTaskSingle(this);
	}

	virtual void Reset() override {
		m_texture = {};
	}

	void Setup(SetupContext& context) override {
		unsigned width = GetInput<0>().Get();
		unsigned height = GetInput<1>().Get();
		gxapi::eFormat format = GetInput<2>().Get();
		uint16_t arrayCount = GetInput<3>().Get();
		TextureUsage usage = GetInput<4>().Get();
		bool isCubemap = GetInput<5>().Get();
		bool mipchainNeeded = GetInput<6>().Get();

		auto UsageToFlags = [](TextureUsage usage) {
			gxapi::eResourceFlags flags;
			if (!usage.shaderResource) flags += gxapi::eResourceFlags::DENY_SHADER_RESOURCE;
			if (usage.renderTarget) flags += gxapi::eResourceFlags::ALLOW_RENDER_TARGET;
			if (usage.depthStencil) flags += gxapi::eResourceFlags::ALLOW_DEPTH_STENCIL;
			if (usage.randomAccess) flags += gxapi::eResourceFlags::ALLOW_UNORDERED_ACCESS;
			return flags;
		};

		gxapi::eResourceFlags flagMask;
		flagMask += gxapi::eResourceFlags::DENY_SHADER_RESOURCE;
		flagMask += gxapi::eResourceFlags::ALLOW_RENDER_TARGET;
		flagMask += gxapi::eResourceFlags::ALLOW_DEPTH_STENCIL;
		flagMask += gxapi::eResourceFlags::ALLOW_UNORDERED_ACCESS;
		gxapi::eResourceFlags requestedFlags = UsageToFlags(usage);

		if (!m_texture.HasObject()
			|| m_texture.GetWidth() != width
			|| m_texture.GetHeight() != height
			|| m_texture.GetFormat() != format
			|| m_texture.GetArrayCount() != arrayCount
			|| (m_texture.GetDescription().textureDesc.flags & flagMask) != requestedFlags)
		{
			m_texture = context.CreateTexture2D(Texture2DDesc{ width, height, format, uint16_t(mipchainNeeded ? getNumMips(width, height, 1) : 1), arrayCount }, usage);
		}

		GetOutput<0>().Set(m_texture);
	}

	void Execute(RenderContext& context) override {}
private:
	gxeng::Texture2D m_texture;
};


} // namespace inl::gxeng::nodes
