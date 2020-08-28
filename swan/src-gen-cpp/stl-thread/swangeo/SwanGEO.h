#include <fstream>
#include <iomanip>
#include <type_traits>
#include <limits>
#include <utility>
#include <cmath>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include "mesh/CartesianMesh2DGenerator.h"
#include "mesh/CartesianMesh2D.h"
#include "utils/Utils.h"
#include "utils/Timer.h"
#include "types/Types.h"
#include "swangeo/SwanGEOFunctions.h"
#include "mesh/stl/PvdFileWriter2D.h"
#include "utils/stl/Parallel.h"

using namespace nablalib;

/******************** Free functions declarations ********************/



/******************** Module declaration ********************/

class SwanGEO
{
public:
	struct Options
	{
		std::string outputPath;
		int outputPeriod;
		double X_EDGE_LENGTH;
		double Y_EDGE_LENGTH;
		int X_EDGE_ELEMS;
		int Y_EDGE_ELEMS;
		double X_LENGTH;
		double Y_LENGTH;
		bool DConst;
		double Dini;
		double deltat;
		int maxIter;
		double stopTime;

		void jsonInit(const rapidjson::Value::ConstObject& d);
	};

	const Options& options;
	SwanGEOFunctions& swanGEOFunctions;

	SwanGEO(const Options& aOptions, SwanGEOFunctions& aSwanGEOFunctions);
	~SwanGEO();

private:
	// Global definitions
	double t_n;
	double t_nplus1;
	static constexpr double deltax_lon = 0.033;
	static constexpr double deltay_lat = 0.033;
	static constexpr double g = -9.8;
	static constexpr double C = 40.0;
	static constexpr double F = 0.0;
	static constexpr double DEG2RAD_DP = 0.01745;
	static constexpr double DEG2RAD = DEG2RAD_DP;
	static constexpr double RME_DP = 6371000.0;
	static constexpr double DEG2M_DP = 111194.9266;
	static constexpr double DEG2M = DEG2M_DP;
	int lastDump;
	
	// Mesh (can depend on previous definitions)
	CartesianMesh2D* mesh;
	PvdFileWriter2D writer;
	size_t nbNodes, nbFaces, nbInnerFaces, nbInnerVerticalFaces, nbInnerHorizontalFaces, nbCells, nbInnerCells, nbOuterCells, nbTopCells, nbBottomCells, nbLeftCells, nbRightCells, nbNodesOfCell;
	
	// Global declarations
	int n;
	std::vector<RealArray1D<2>> X;
	std::vector<RealArray1D<2>> Xc;
	std::vector<double> xc;
	std::vector<double> yc;
	std::vector<double> deltax;
	std::vector<double> deltay;
	std::vector<double> U_n;
	std::vector<double> U_nplus1;
	std::vector<double> U_n0;
	std::vector<double> Uini;
	std::vector<double> V_n;
	std::vector<double> V_nplus1;
	std::vector<double> V_n0;
	std::vector<double> Vini;
	std::vector<double> H_n;
	std::vector<double> H_nplus1;
	std::vector<double> H_n0;
	std::vector<double> Hini;
	std::vector<double> Rijini;
	std::vector<double> Rij_n;
	std::vector<double> Rij_nplus1;
	std::vector<double> Rij_n0;
	std::vector<double> Fx;
	std::vector<double> Fy;
	std::vector<double> Dijini;
	std::vector<double> Dij;
	std::vector<double> Bool;
	utils::Timer globalTimer;
	utils::Timer cpuTimer;
	utils::Timer ioTimer;

	void computeTn() noexcept;
	
	void initDijini() noexcept;
	
	void initFxy() noexcept;
	
	void initHini() noexcept;
	
	void initRijini() noexcept;
	
	void initUini() noexcept;
	
	void initVini() noexcept;
	
	void initXc() noexcept;
	
	void updateHinner() noexcept;
	
	void updateHouter() noexcept;
	
	void updateRij() noexcept;
	
	void updateUinner() noexcept;
	
	void updateUouter() noexcept;
	
	void updateVinner() noexcept;
	
	void updateVouter() noexcept;
	
	void initDij() noexcept;
	
	void initH() noexcept;
	
	void initRij() noexcept;
	
	void initU() noexcept;
	
	void initV() noexcept;
	
	void initXcAndYc() noexcept;
	
	void initBool() noexcept;
	
	void initdeltaxdeltay() noexcept;
	
	void setUpTimeLoopN() noexcept;
	
	void executeTimeLoopN() noexcept;

	void dumpVariables(int iteration, bool useTimer=true);

public:
	void simulate();
};
