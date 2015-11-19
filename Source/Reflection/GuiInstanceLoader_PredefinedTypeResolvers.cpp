#include "GuiInstanceLoader.h"
#include "TypeDescriptors/GuiReflectionEvents.h"
#include "../Resources/GuiParserManager.h"
#include "InstanceQuery/GuiInstanceQuery.h"
#include "GuiInstanceSharedScript.h"
#include "GuiInstanceLoader_WorkflowCompiler.h"

namespace vl
{
	namespace presentation
	{
		using namespace parsing;
		using namespace parsing::xml;
		using namespace workflow::analyzer;
		using namespace workflow::runtime;
		using namespace reflection::description;
		using namespace collections;

#define ERROR_CODE_PREFIX L"================================================================"

/***********************************************************************
Instance Type Resolver
***********************************************************************/

		class GuiResourceInstanceTypeResolver
			: public Object
			, public IGuiResourceTypeResolver
			, private IGuiResourceTypeResolver_Precompile
			, private IGuiResourceTypeResolver_DirectLoadStream
			, private IGuiResourceTypeResolver_IndirectLoad
		{
		public:
			WString GetType()override
			{
				return L"Instance";
			}

			WString GetPreloadType()override
			{
				return L"Xml";
			}

			bool IsDelayLoad()override
			{
				return false;
			}

			vint GetMaxPassIndex()override
			{
				return 2;
			}

			void Precompile(Ptr<DescriptableObject> resource, GuiResource* rootResource, vint passIndex, Ptr<GuiResourcePathResolver> resolver, collections::List<WString>& errors)override
			{
				if (passIndex == 2)
				{
					if (auto obj = resource.Cast<GuiInstanceContext>())
					{
						obj->ApplyStyles(resolver, errors);
						Workflow_PrecompileInstanceContext(obj, errors);
					}
				}
			}

			IGuiResourceTypeResolver_Precompile* Precompile()override
			{
				return this;
			}

			IGuiResourceTypeResolver_DirectLoadStream* DirectLoadStream()override
			{
				return this;
			}

			IGuiResourceTypeResolver_IndirectLoad* IndirectLoad()override
			{
				return this;
			}

			void SerializePrecompiled(Ptr<DescriptableObject> resource, stream::IStream& stream)override
			{
				auto obj = resource.Cast<GuiInstanceContext>();
				obj->SavePrecompiledBinary(stream);
			}

			Ptr<DescriptableObject> ResolveResourcePrecompiled(stream::IStream& stream, collections::List<WString>& errors)override
			{
				return GuiInstanceContext::LoadPrecompiledBinary(stream, errors);
			}

			Ptr<DescriptableObject> Serialize(Ptr<DescriptableObject> resource, bool serializePrecompiledResource)override
			{
				if (auto obj = resource.Cast<GuiInstanceContext>())
				{
					return obj->SaveToXml(serializePrecompiledResource);
				}
				return 0;
			}

			Ptr<DescriptableObject> ResolveResource(Ptr<DescriptableObject> resource, Ptr<GuiResourcePathResolver> resolver, collections::List<WString>& errors)override
			{
				Ptr<XmlDocument> xml = resource.Cast<XmlDocument>();
				if (xml)
				{
					Ptr<GuiInstanceContext> context = GuiInstanceContext::LoadFromXml(xml, errors);
					return context;
				}
				return 0;
			}
		};

/***********************************************************************
Instance Style Type Resolver
***********************************************************************/

		class GuiResourceInstanceStyleResolver
			: public Object
			, public IGuiResourceTypeResolver
			, private IGuiResourceTypeResolver_IndirectLoad
		{
		public:
			WString GetType()override
			{
				return L"InstanceStyle";
			}

			WString GetPreloadType()override
			{
				return L"Xml";
			}

			bool IsDelayLoad()override
			{
				return false;
			}

			IGuiResourceTypeResolver_IndirectLoad* IndirectLoad()override
			{
				return this;
			}

			Ptr<DescriptableObject> Serialize(Ptr<DescriptableObject> resource, bool serializePrecompiledResource)override
			{
				if (!serializePrecompiledResource)
				{
					if (auto obj = resource.Cast<GuiInstanceStyleContext>())
					{
						return obj->SaveToXml();
					}
				}
				return 0;
			}

			Ptr<DescriptableObject> ResolveResource(Ptr<DescriptableObject> resource, Ptr<GuiResourcePathResolver> resolver, collections::List<WString>& errors)override
			{
				Ptr<XmlDocument> xml = resource.Cast<XmlDocument>();
				if (xml)
				{
					auto context = GuiInstanceStyleContext::LoadFromXml(xml, errors);
					return context;
				}
				return 0;
			}
		};

/***********************************************************************
Shared Script Type Resolver
***********************************************************************/

		class GuiResourceSharedScriptTypeResolver
			: public Object
			, public IGuiResourceTypeResolver
			, private IGuiResourceTypeResolver_Precompile
			, private IGuiResourceTypeResolver_IndirectLoad
		{
		public:
			WString GetType()override
			{
				return L"Script";
			}

			WString GetPreloadType()override
			{
				return L"Xml";
			}

			bool IsDelayLoad()override
			{
				return false;
			}

			vint GetMaxPassIndex()override
			{
				return 1;
			}

			void Precompile(Ptr<DescriptableObject> resource, GuiResource* rootResource, vint passIndex, Ptr<GuiResourcePathResolver> resolver, collections::List<WString>& errors)override
			{
				if (passIndex == 0)
				{
					if (auto obj = resource.Cast<GuiInstanceSharedScript>())
					{
						if (obj->language == L"Workflow")
						{
							/*
							cache->moduleCodes.Add(obj->code);
							*/
						}
					}
				}
				else if (passIndex == 1)
				{
					/*
					auto table = GetParserManager()->GetParsingTable(L"WORKFLOW");
					List<WString> moduleCodes;
					List<Ptr<ParsingError>> scriptErrors;
					auto assembly = Compile(table, moduleCodes, scriptErrors);

					if (scriptErrors.Count() > 0)
					{
						errors.Add(ERROR_CODE_PREFIX L"Failed to parse the shared workflow script");
						FOREACH(Ptr<ParsingError>, error, scriptErrors)
						{
							errors.Add(
								L"Row: " + itow(error->codeRange.start.row + 1) +
								L", Column: " + itow(error->codeRange.start.column + 1) +
								L", Message: " + error->errorMessage);
						}
					}
					else
					{
						cache->Initialize();
					}
					*/
				}
			}

			IGuiResourceTypeResolver_Precompile* Precompile()override
			{
				return this;
			}

			IGuiResourceTypeResolver_IndirectLoad* IndirectLoad()override
			{
				return this;
			}

			Ptr<DescriptableObject> Serialize(Ptr<DescriptableObject> resource, bool serializePrecompiledResource)override
			{
				if (!serializePrecompiledResource)
				{
					if (auto obj = resource.Cast<GuiInstanceSharedScript>())
					{
						return obj->SaveToXml();
					}
				}
				return 0;
			}

			Ptr<DescriptableObject> ResolveResource(Ptr<DescriptableObject> resource, Ptr<GuiResourcePathResolver> resolver, collections::List<WString>& errors)override
			{
				Ptr<XmlDocument> xml = resource.Cast<XmlDocument>();
				if (xml)
				{
					auto schema = GuiInstanceSharedScript::LoadFromXml(xml, errors);
					return schema;
				}
				return 0;
			}
		};

/***********************************************************************
Shared Script Type Resolver
***********************************************************************/

		class GuiPredefinedTypeResolversPlugin : public Object, public IGuiPlugin
		{
		public:
			GuiPredefinedTypeResolversPlugin()
			{
			}

			void Load()override
			{
			}

			void AfterLoad()override
			{
				IGuiResourceResolverManager* manager = GetResourceResolverManager();
				manager->SetTypeResolver(new GuiResourceInstanceTypeResolver);
				manager->SetTypeResolver(new GuiResourceInstanceStyleResolver);
				manager->SetTypeResolver(new GuiResourceSharedScriptTypeResolver);
			}

			void Unload()override
			{
			}
		};
		GUI_REGISTER_PLUGIN(GuiPredefinedTypeResolversPlugin)
	}
}