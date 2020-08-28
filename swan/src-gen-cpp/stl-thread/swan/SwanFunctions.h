#include <string>
#include <rapidjson/document.h>
#include "netcdf.h"

class SwanFunctions
{
public:
	void jsonInitWave(const rapidjson::Value::ConstObject& d)
	{
		// if the option is mandatory, the line should be:
		// assert(d.HasMember("fileName"));
		if (d.HasMember("waveFilename"))
		{
			const rapidjson::Value& valueof_fileName = d["waveFilename"];
			assert(valueof_fileName.IsString());
			waveFilename = valueof_fileName.GetString();
		}
		if (d.HasMember("waveVarName"))
		{
			const rapidjson::Value& valueof_fileName = d["waveVarName"];
			assert(valueof_fileName.IsString());
			waveVarName = valueof_fileName.GetString();
		}
	}

	void jsonInitDepth(const rapidjson::Value::ConstObject& d)
		{
			// if the option is mandatory, the line should be:
			// assert(d.HasMember("fileName"));
			if (d.HasMember("depthFilename"))
			{
				const rapidjson::Value& valueof_fileName = d["depthFilename"];
				assert(valueof_fileName.IsString());
				depthFilename = valueof_fileName.GetString();
			}
			if (d.HasMember("depthVarName"))
			{
				const rapidjson::Value& valueof_fileName = d["depthVarName"];
				assert(valueof_fileName.IsString());
				depthVarName = valueof_fileName.GetString();
			}
		}

	void jsonInitLon(const rapidjson::Value::ConstObject& d)
		{
			// if the option is mandatory, the line should be:
			// assert(d.HasMember("fileName"));
			if (d.HasMember("lonFilename"))
			{
				const rapidjson::Value& valueof_fileName = d["lonFilename"];
				assert(valueof_fileName.IsString());
				lonFilename = valueof_fileName.GetString();
			}
			if (d.HasMember("lonVarName"))
			{
				const rapidjson::Value& valueof_fileName = d["lonVarName"];
				assert(valueof_fileName.IsString());
				lonVarName = valueof_fileName.GetString();
			}
		}
	void jsonInitLat(const rapidjson::Value::ConstObject& d)
		{
		// if the option is mandatory, the line should be:
		// assert(d.HasMember("fileName"));
			if (d.HasMember("latFilename"))
			{
				const rapidjson::Value& valueof_fileName = d["latFilename"];
				assert(valueof_fileName.IsString());
				latFilename = valueof_fileName.GetString();
			}
			if (d.HasMember("latVarName"))
			{
				const rapidjson::Value& valueof_fileName = d["latVarName"];
				assert(valueof_fileName.IsString());
				latVarName = valueof_fileName.GetString();
			}
		}

	void jsonInitBottomMotion(const rapidjson::Value::ConstObject& d)
		{
		// if the option is mandatory, the line should be:
		// assert(d.HasMember("fileName"));
			if (d.HasMember("bottomMotionFilename"))
			{
				const rapidjson::Value& valueof_fileName = d["bottomMotionFilename"];
				assert(valueof_fileName.IsString());
				bottomMotionFilename = valueof_fileName.GetString();
			}
			if (d.HasMember("bottomVarName"))
			{
				const rapidjson::Value& valueof_fileName = d["bottomVarName"];
				assert(valueof_fileName.IsString());
				bottomVarName = valueof_fileName.GetString();
			}
		}

	void jsonInit(const rapidjson::Value::ConstObject& d)
	{
		jsonInitDepth(d);
		jsonInitLon(d);
		jsonInitLat(d);
		jsonInitWave(d);
		jsonInitBottomMotion(d);
	}

	float nextWaveHeight();
	float nextDepth();
	float nextBottomMotion();
	float nextLon();
	float nextLat();

private:
	float* dataWave = NULL;
	unsigned int countWave = 0;
	unsigned int countWaveMax = 1;
	float waveHeight = 0;
	std::string waveFilename = "";
	std::string waveVarName = "";

	float* dataBottom = NULL;
	unsigned int countBottom = 0;
	unsigned int countBottomMax = 1;
	float bottom = 0;
	std::string bottomMotionFilename = "";
	std::string bottomVarName = "";

	float* dataDepth = NULL;
	unsigned int countDepth = 0;
	unsigned int countDepthMax = 1;
	float depth = 0;
	std::string depthFilename = "";
	std::string depthVarName = "";

	float* dataLon = NULL;
	unsigned int countLon = 0;
	unsigned int countLonMax = 1;
	float lon = 0;
	std::string lonFilename = "";
	std::string lonVarName = "";

	float* dataLat = NULL;
	unsigned int countLat = 0;
	unsigned int countLatMax = 1;
	float lat = 0;
	std::string latFilename = "";
	std::string latVarName = "";
};
