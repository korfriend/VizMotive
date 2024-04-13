#pragma once
#define __dojostatic extern "C" __declspec(dllexport)
#define __dojoclass class //__declspec(dllexport)
#define __dojostruct struct __declspec(dllexport)

#define __FP (float*)&
#define VZRESULT int
#define VZ_OK 0
#define VZ_FAIL 1
#define VZ_JOB_WAIT 1

#define SAFE_GET_COPY(DST_PTR, SRC_PTR, TYPE, ELEMENTS) { if(DST_PTR) memcpy(DST_PTR, SRC_PTR, sizeof(TYPE)*ELEMENTS); }
#define GET_COPY(DST_PTR, SRC_PTR, TYPE, ELEMENTS) { memcpy(DST_PTR, SRC_PTR, sizeof(TYPE)*ELEMENTS); }

// std
#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <set>
#include <any>
#include <list>
#include <memory>
#include <algorithm>
#include <chrono>
#include <functional>

using VID = uint32_t;
inline constexpr VID INVALID_VID = 0;
using TimeStamp = std::chrono::high_resolution_clock::time_point;

constexpr float VZ_PI = 3.141592654f;
constexpr float VZ_2PI = 6.283185307f;
constexpr float VZ_1DIVPI = 0.318309886f;
constexpr float VZ_1DIV2PI = 0.159154943f;
constexpr float VZ_PIDIV2 = 1.570796327f;
constexpr float VZ_PIDIV4 = 0.785398163f;

using uint = uint32_t;

// DOJO TO DO (or suggestion)
// 1. separate mesh and material (mesh to geometry... and material is an option for objectcomponent)
// 2. set up a resource pool (or a scene for this?! for the geometry anbd material, animations,....)

namespace vzm
{
	__dojostatic inline void TransformPoint(const float posSrc[3], const float mat[16], const bool rowMajor, float posDst[3]);
	__dojostatic inline void TransformVector(const float vecSrc[3], const float mat[16], const bool rowMajor, float vecDst[3]);
	__dojostatic inline void ComputeBoxTransformMatrix(const float cubeScale[3], const float posCenter[3],
		const float yAxis[3], const float zAxis[3], const bool rowMajor, float mat[16], float matInv[16]);

	template <typename ID> struct ParamMap {
	private:
		std::string __PM_VERSION = "LIBI_1.4";
		std::unordered_map<ID, std::any> __params;
	public:
		bool FindParam(const ID& param_name) {
			auto it = __params.find(param_name);
			return !(it == __params.end());
		}
		template <typename SRCV> bool GetParamCheck(const ID& key, SRCV& param) {
			auto it = __params.find(key);
			if (it == __params.end()) return false;
			param = std::any_cast<SRCV&>(it->second);
			return true;
		}
		template <typename SRCV> SRCV GetParam(const ID& key, const SRCV& init_v) const {
			auto it = __params.find(key);
			if (it == __params.end()) return init_v;
			return std::any_cast<const SRCV&>(it->second);
		}
		template <typename SRCV> SRCV* GetParamPtr(const ID& key) {
			auto it = __params.find(key);
			if (it == __params.end()) return NULL;
			return (SRCV*)&std::any_cast<SRCV&>(it->second);
		}
		template <typename SRCV, typename DSTV> bool GetParamCastingCheck(const ID& key, DSTV& param) {
			auto it = __params.find(key);
			if (it == __params.end()) return false;
			param = (DSTV)std::any_cast<SRCV&>(it->second);
			return true;
		}
		template <typename SRCV, typename DSTV> DSTV GetParamCasting(const ID& key, const DSTV& init_v) {
			auto it = __params.find(key);
			if (it == __params.end()) return init_v;
			return (DSTV)std::any_cast<SRCV&>(it->second);
		}
		void SetParam(const ID& key, const std::any& param) {
			__params[key] = param;
		}
		void RemoveParam(const ID& key) {
			auto it = __params.find(key);
			if (it != __params.end()) {
				__params.erase(it);
			}
		}
		void RemoveAll() {
			__params.clear();
		}
		size_t Size() {
			return __params.size();
		}
		std::string GetPMapVersion() {
			return __PM_VERSION;
		}

		typedef std::unordered_map<ID, std::any> MapType;
		typename typedef MapType::iterator iterator;
		typename typedef MapType::const_iterator const_iterator;
		typename typedef MapType::reference reference;
		iterator begin() { return __params.begin(); }
		const_iterator begin() const { return __params.begin(); }
		iterator end() { return __params.end(); }
		const_iterator end() const { return __params.end(); }
	};

	enum class COMPONENT_TYPE
	{
		UNDEFINED = 0,
		CAMERA,
		ACTOR,
		GEOMETRY,
		MATERIAL,
		LIGHT,
		EMITTER,
		ANIMATION,
		WEATHER,
	};

	struct VmBaseComponent
	{
		VID componentVID = INVALID_VID;
		COMPONENT_TYPE compType = COMPONENT_TYPE::UNDEFINED;
		TimeStamp timeStamp = {}; // will be automatically set 
		ParamMap<std::string> attributes;

		void GetLocalTransform(float mat[16], const bool rowMajor = false);
		void GetWorldTransform(float mat[16], const bool rowMajor = false);
		void GetLocalInvTransform(float mat[16], const bool rowMajor = false);
		void GetWorldInvTransform(float mat[16], const bool rowMajor = false);
	};
	struct VmRenderer;
	struct VmCamera : VmBaseComponent
	{
		VmRenderer* renderer = nullptr;
		void SetPose(const float pos[3], const float view[3], const float up[3]);
		void SetPerspectiveProjection(const float zNearP, const float zFarP, const float fovY, const float aspectRatio);
		void SetCanvasSize(const float w, const float h, const float dpi);
		void GetPose(float pos[3], float view[3], float up[3]);
		void GetPerspectiveProjection(float* zNearP, float* zFarP, float* fovY, float* aspectRatio);
		void GetCanvasSize(float* w, float* h, float* dpi);
	};
	struct VmActor : VmBaseComponent
	{
		// mesh 에 rendering 요소가 저장된 구조...
	};
	struct VmMesh : VmBaseComponent
	{
		// mesh 가 material entity 를 갖는 구조...
	};
	struct VmMaterial : VmBaseComponent
	{
	};
	struct VmEmitter : VmBaseComponent
	{
		struct ParticleCounters
		{
			uint aliveCount;
			uint deadCount;
			uint realEmitCount;
			uint aliveCount_afterSimulation;
			uint culledCount;
			uint cellAllocator;
		};

		enum PARTICLESHADERTYPE
		{
			SOFT,
			SOFT_DISTORTION,
			SIMPLE,
			SOFT_LIGHTING,
			PARTICLESHADERTYPE_COUNT,
			ENUM_FORCE_UINT32 = 0xFFFFFFFF,
		};

		void Burst(int num);
		void Restart();

		void GetStatistics(ParticleCounters& statistics);

		PARTICLESHADERTYPE GetShaderType();
		void SetShaderType(const PARTICLESHADERTYPE shaderType);

		VID GetMeshVid();
		void SetMeshVid(const VID vid);

		// -1 : variable timestep; >=0 : fixed timestep
		float GetFixedTimeStep();
		void SetFixedTimeStep(const float FIXED_TIMESTEP);

		float GetSize();
		void SetSize(const float size);
		float GetRandomFactor();
		void SetRandomFactor(const float random_factor);
		float GetNormalFactor();
		void SetNormalFactor(const float normal_factor);
		float GetCount();
		void SetCount(const float count);
		float GetLife();
		void SetLife(const float life);
		float GetRandomLife();
		void SetRandomLife(const float random_life);
		void GetScaleXY(float* scaleX, float* scaleY);
		void SetScaleXY(const float scaleX, const float scaleY);
		float GetRotation();
		void SetRotation(const float rotation);
		float GetMotionBlurAmount();
		void SetMotionBlurAmount(const float motionBlurAmount);
		float GetMass();
		void SetMass(const float mass);
		float GetRandomColor();
		void SetRandomColor(const float random_color);

		void GetVelocity(float velocity[3]);
		void SetVelocity(const float velocity[3]);
		void GetGravity(float gravity[3]);
		void SetGravity(const float gravity[3]);
		float GetDrag();
		void SetDrag(const float drag);
		float GetRestitution();
		void SetRestitution(const float restitution);

		// smoothing radius, pressure constant, reference density, viscosity constant
		void GetSPHProps(float* SPH_h, float* SPH_K, float* SPH_p0, float* SPH_e);
		void SetSPHProps(const float SPH_h, const float SPH_K, const float SPH_p0, const float SPH_e);

		void GetSpriteSheetProps(uint32_t* framesX, uint32_t* framesY, uint32_t* frameCount, uint32_t* frameStart);
		void SetSpriteSheetProps(const uint32_t framesX, const uint32_t framesY, const uint32_t frameCount, const uint32_t frameStart);

		// Core Component Intrinsics
		void SetMaxParticleCount(uint32_t value);
		uint32_t GetMaxParticleCount();
		uint64_t GetMemorySizeInBytes();

		bool IsDebug();
		bool IsPaused();
		bool IsSorted();
		bool IsDepthCollisionEnabled();
		bool IsSPHEnabled();
		bool IsVolumeEnabled();
		bool IsFrameBlendingEnabled();
		bool IsCollidersDisabled();
		bool IsTakeColorFromMesh();

		void SetDebug(const bool value);
		void SetPaused(const bool value);
		void SetSorted(const bool value);
		void SetDepthCollisionEnabled(const bool value);
		void SetSPHEnabled(const bool value);
		void SetVolumeEnabled(const bool value);
		void SetFrameBlendingEnabled(const bool value);
		void SetCollidersDisabled(const bool value);
		void SetTakeColorFromMesh(const bool value);
	};
	struct VmLight : VmBaseComponent
	{
	};
	struct VmWeather : VmBaseComponent
	{
		void SetWeatherPreset(const uint32_t index);
	};
	struct VmAnimation : VmBaseComponent
	{
		void Play();
		void Pause();
		void Stop();
		void SetLooped(const bool value);
	};
}
