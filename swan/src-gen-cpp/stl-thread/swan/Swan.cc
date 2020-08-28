#include "swan/Swan.h"

using namespace nablalib;

/******************** Free functions definitions ********************/

template<size_t x>
RealArray1D<x> sumR1(RealArray1D<x> a, RealArray1D<x> b)
{
	return a + b;
}


/******************** Options definition ********************/

void
Swan::Options::jsonInit(const rapidjson::Value::ConstObject& d)
{
	// outputPath
	assert(d.HasMember("outputPath"));
	const rapidjson::Value& valueof_outputPath = d["outputPath"];
	assert(valueof_outputPath.IsString());
	outputPath = valueof_outputPath.GetString();
	// outputPeriod
	assert(d.HasMember("outputPeriod"));
	const rapidjson::Value& valueof_outputPeriod = d["outputPeriod"];
	assert(valueof_outputPeriod.IsInt());
	outputPeriod = valueof_outputPeriod.GetInt();
	// X_EDGE_LENGTH
	assert(d.HasMember("X_EDGE_LENGTH"));
	const rapidjson::Value& valueof_X_EDGE_LENGTH = d["X_EDGE_LENGTH"];
	assert(valueof_X_EDGE_LENGTH.IsDouble());
	X_EDGE_LENGTH = valueof_X_EDGE_LENGTH.GetDouble();
	// Y_EDGE_LENGTH
	assert(d.HasMember("Y_EDGE_LENGTH"));
	const rapidjson::Value& valueof_Y_EDGE_LENGTH = d["Y_EDGE_LENGTH"];
	assert(valueof_Y_EDGE_LENGTH.IsDouble());
	Y_EDGE_LENGTH = valueof_Y_EDGE_LENGTH.GetDouble();
	// X_EDGE_ELEMS
	assert(d.HasMember("X_EDGE_ELEMS"));
	const rapidjson::Value& valueof_X_EDGE_ELEMS = d["X_EDGE_ELEMS"];
	assert(valueof_X_EDGE_ELEMS.IsInt());
	X_EDGE_ELEMS = valueof_X_EDGE_ELEMS.GetInt();
	// Y_EDGE_ELEMS
	assert(d.HasMember("Y_EDGE_ELEMS"));
	const rapidjson::Value& valueof_Y_EDGE_ELEMS = d["Y_EDGE_ELEMS"];
	assert(valueof_Y_EDGE_ELEMS.IsInt());
	Y_EDGE_ELEMS = valueof_Y_EDGE_ELEMS.GetInt();
	// X_LENGTH
	assert(d.HasMember("X_LENGTH"));
	const rapidjson::Value& valueof_X_LENGTH = d["X_LENGTH"];
	assert(valueof_X_LENGTH.IsDouble());
	X_LENGTH = valueof_X_LENGTH.GetDouble();
	// Y_LENGTH
	assert(d.HasMember("Y_LENGTH"));
	const rapidjson::Value& valueof_Y_LENGTH = d["Y_LENGTH"];
	assert(valueof_Y_LENGTH.IsDouble());
	Y_LENGTH = valueof_Y_LENGTH.GetDouble();
	// DConst
	assert(d.HasMember("DConst"));
	const rapidjson::Value& valueof_DConst = d["DConst"];
	assert(valueof_DConst.IsBool());
	DConst = valueof_DConst.GetBool();
	// Dini
	assert(d.HasMember("Dini"));
	const rapidjson::Value& valueof_Dini = d["Dini"];
	assert(valueof_Dini.IsDouble());
	Dini = valueof_Dini.GetDouble();
	// deltat
	assert(d.HasMember("deltat"));
	const rapidjson::Value& valueof_deltat = d["deltat"];
	assert(valueof_deltat.IsDouble());
	deltat = valueof_deltat.GetDouble();
	// F
	assert(d.HasMember("F"));
	const rapidjson::Value& valueof_F = d["F"];
	assert(valueof_F.IsDouble());
	F = valueof_F.GetDouble();
	// maxIter
	assert(d.HasMember("maxIter"));
	const rapidjson::Value& valueof_maxIter = d["maxIter"];
	assert(valueof_maxIter.IsInt());
	maxIter = valueof_maxIter.GetInt();
	// stopTime
	assert(d.HasMember("stopTime"));
	const rapidjson::Value& valueof_stopTime = d["stopTime"];
	assert(valueof_stopTime.IsDouble());
	stopTime = valueof_stopTime.GetDouble();
}

/******************** Module definition ********************/

Swan::Swan(const Options& aOptions, SwanFunctions& aSwanFunctions)
: options(aOptions)
, swanFunctions(aSwanFunctions)
, t_n(0.0)
, t_nplus1(0.0)
, deltax(options.X_EDGE_LENGTH)
, deltay(options.Y_EDGE_LENGTH)
, lastDump(numeric_limits<int>::min())
, mesh(CartesianMesh2DGenerator::generate(options.X_EDGE_ELEMS, options.Y_EDGE_ELEMS, options.X_EDGE_LENGTH, options.Y_EDGE_LENGTH))
, writer("Swan", options.outputPath)
, nbNodes(mesh->getNbNodes())
, nbFaces(mesh->getNbFaces())
, nbInnerFaces(mesh->getNbInnerFaces())
, nbInnerVerticalFaces(mesh->getNbInnerVerticalFaces())
, nbInnerHorizontalFaces(mesh->getNbInnerHorizontalFaces())
, nbCells(mesh->getNbCells())
, nbInnerCells(mesh->getNbInnerCells())
, nbTopCells(mesh->getNbTopCells())
, nbBottomCells(mesh->getNbBottomCells())
, nbLeftCells(mesh->getNbLeftCells())
, nbRightCells(mesh->getNbRightCells())
, nbNodesOfCell(CartesianMesh2D::MaxNbNodesOfCell)
, X(nbNodes)
, Xc(nbCells)
, xc(nbCells)
, yc(nbCells)
, U_n(nbFaces)
, U_nplus1(nbFaces)
, U_n0(nbFaces)
, Uini(nbFaces)
, V_n(nbFaces)
, V_nplus1(nbFaces)
, V_n0(nbFaces)
, Vini(nbFaces)
, H_n(nbCells)
, H_nplus1(nbCells)
, H_n0(nbCells)
, Hini(nbCells)
, Rijini(nbCells)
, Rij_n(nbCells)
, Rij_nplus1(nbCells)
, Rij_n0(nbCells)
, Fx(nbCells)
, Fy(nbCells)
, Dijini(nbCells)
, Dij(nbCells)
, Bool(nbCells)
{
	// Copy node coordinates
	const auto& gNodes = mesh->getGeometry()->getNodes();
	for (size_t rNodes=0; rNodes<nbNodes; rNodes++)
	{
		X[rNodes][0] = gNodes[rNodes][0];
		X[rNodes][1] = gNodes[rNodes][1];
	}
}

Swan::~Swan()
{
	delete mesh;
}

/**
 * Job ComputeTn called @1.0 in executeTimeLoopN method.
 * In variables: deltat, t_n
 * Out variables: t_nplus1
 */
void Swan::computeTn() noexcept
{
	t_nplus1 = t_n + options.deltat;
}

/**
 * Job InitDijini called @1.0 in simulate method.
 * In variables: DConst, Dini
 * Out variables: Dijini
 */
void Swan::initDijini() noexcept
{
	if (options.DConst) 
	{
		const auto innerCells(mesh->getInnerCells());
		const size_t nbInnerCells(innerCells.size());
		parallel::parallel_exec(nbInnerCells, [&](const size_t& icInnerCells)
		{
			const Id icId(innerCells[icInnerCells]);
			const size_t icCells(icId);
			Dijini[icCells] = options.Dini;
		});
	}
	else
	{
		const auto innerCells(mesh->getInnerCells());
		const size_t nbInnerCells(innerCells.size());
		for (size_t icInnerCells=0; icInnerCells<nbInnerCells; icInnerCells++)
		{
			const Id icId(innerCells[icInnerCells]);
			const size_t icCells(icId);
			Dijini[icCells] = swanFunctions.nextDepth();
		}
	}
}

/**
 * Job InitFxy called @1.0 in simulate method.
 * In variables: 
 * Out variables: Fx, Fy
 */
void Swan::initFxy() noexcept
{
	parallel::parallel_exec(nbCells, [&](const size_t& cCells)
	{
		Fx[cCells] = 0.0;
		Fy[cCells] = 0.0;
	});
}

/**
 * Job InitRijini called @1.0 in simulate method.
 * In variables: 
 * Out variables: Rijini
 */
void Swan::initRijini() noexcept
{
	{
		const auto innerCells(mesh->getInnerCells());
		const size_t nbInnerCells(innerCells.size());
		parallel::parallel_exec(nbInnerCells, [&](const size_t& icInnerCells)
		{
			const Id icId(innerCells[icInnerCells]);
			const size_t icCells(icId);
			Rijini[icCells] = 0.0;
		});
	}
}

/**
 * Job InitUini called @1.0 in simulate method.
 * In variables: 
 * Out variables: Uini
 */
void Swan::initUini() noexcept
{
	{
		const auto innerFaces(mesh->getInnerFaces());
		const size_t nbInnerFaces(innerFaces.size());
		parallel::parallel_exec(nbInnerFaces, [&](const size_t& fInnerFaces)
		{
			const Id fId(innerFaces[fInnerFaces]);
			const size_t fFaces(fId);
			Uini[fFaces] = 0.0;
		});
	}
}

/**
 * Job InitVini called @1.0 in simulate method.
 * In variables: 
 * Out variables: Vini
 */
void Swan::initVini() noexcept
{
	{
		const auto innerFaces(mesh->getInnerFaces());
		const size_t nbInnerFaces(innerFaces.size());
		parallel::parallel_exec(nbInnerFaces, [&](const size_t& fInnerFaces)
		{
			const Id fId(innerFaces[fInnerFaces]);
			const size_t fFaces(fId);
			Vini[fFaces] = 0.0;
		});
	}
}

/**
 * Job InitXc called @1.0 in simulate method.
 * In variables: X
 * Out variables: Xc
 */
void Swan::initXc() noexcept
{
	parallel::parallel_exec(nbCells, [&](const size_t& cCells)
	{
		const Id cId(cCells);
		RealArray1D<2> reduction0({0.0, 0.0});
		{
			const auto nodesOfCellC(mesh->getNodesOfCell(cId));
			const size_t nbNodesOfCellC(nodesOfCellC.size());
			for (size_t pNodesOfCellC=0; pNodesOfCellC<nbNodesOfCellC; pNodesOfCellC++)
			{
				const Id pId(nodesOfCellC[pNodesOfCellC]);
				const size_t pNodes(pId);
				reduction0 = sumR1(reduction0, X[pNodes]);
			}
		}
		Xc[cCells] = 0.25 * reduction0;
	});
}

/**
 * Job UpdateHinner called @1.0 in executeTimeLoopN method.
 * In variables: Bool, Dij, H_n, Rij_n, Rij_nplus1, U_n, V_n, deltat, deltax, deltay
 * Out variables: H_nplus1
 */
void Swan::updateHinner() noexcept
{
	{
		const auto innerCells(mesh->getInnerCells());
		const size_t nbInnerCells(innerCells.size());
		parallel::parallel_exec(nbInnerCells, [&](const size_t& icInnerCells)
		{
			const Id icId(innerCells[icInnerCells]);
			const size_t icCells(icId);
			double TD1(0.0);
			double TD2(0.0);
			double TV1(0.0);
			double TV2(0.0);
			{
				const Id rfId(mesh->getRightFaceOfCell(icId));
				const size_t rfFaces(rfId);
				if (U_n[rfFaces] < 0) 
				{
					const Id rcId(mesh->getRightCell(icId));
					const size_t rcCells(rcId);
					TD1 = Dij[rcCells] + H_n[rcCells] - Rij_n[rcCells];
				}
				else
					TD1 = Dij[icCells] + H_n[icCells] - Rij_n[icCells];
			}
			{
				const Id lfId(mesh->getLeftFaceOfCell(icId));
				const size_t lfFaces(lfId);
				if (U_n[lfFaces] < 0) 
					TD2 = Dij[icCells] + H_n[icCells] - Rij_n[icCells];
				else
				{
					const Id lcId(mesh->getLeftCell(icId));
					const size_t lcCells(lcId);
					TD2 = Dij[lcCells] + H_n[lcCells] - Rij_n[lcCells];
				}
			}
			{
				const Id tfId(mesh->getTopFaceOfCell(icId));
				const size_t tfFaces(tfId);
				if (V_n[tfFaces] < 0) 
				{
					const Id tcId(mesh->getTopCell(icId));
					const size_t tcCells(tcId);
					TV1 = Dij[tcCells] + H_n[tcCells] - Rij_n[tcCells];
				}
				else
					TV1 = Dij[icCells] + H_n[icCells] - Rij_n[icCells];
			}
			{
				const Id bfId(mesh->getBottomFaceOfCell(icId));
				const size_t bfFaces(bfId);
				if (V_n[bfFaces] < 0) 
					TV2 = Dij[icCells] + H_n[icCells] - Rij_n[icCells];
				else
				{
					const Id bcId(mesh->getBottomCell(icId));
					const size_t bcCells(bcId);
					TV2 = Dij[bcCells] + H_n[bcCells] - Rij_n[bcCells];
				}
			}
			{
				const Id rfId(mesh->getRightFaceOfCell(icId));
				const size_t rfFaces(rfId);
				const Id lfId(mesh->getLeftFaceOfCell(icId));
				const size_t lfFaces(lfId);
				const Id tfId(mesh->getTopFaceOfCell(icId));
				const size_t tfFaces(tfId);
				const Id bfId(mesh->getBottomFaceOfCell(icId));
				const size_t bfFaces(bfId);
				H_nplus1[icCells] = (H_n[icCells] - options.deltat * (U_n[rfFaces] * TD1 / deltax - U_n[lfFaces] * TD2 / deltax + V_n[tfFaces] * TV1 / deltay - V_n[bfFaces] * TV2 / deltay) + Rij_nplus1[icCells] - Rij_n[icCells]) * Bool[icCells];
			}
		});
	}
}

/**
 * Job UpdateHouter called @1.0 in executeTimeLoopN method.
 * In variables: H_n
 * Out variables: H_nplus1
 */
void Swan::updateHouter() noexcept
{
	{
		const auto topCells(mesh->getTopCells());
		const size_t nbTopCells(topCells.size());
		parallel::parallel_exec(nbTopCells, [&](const size_t& tcTopCells)
		{
			const Id tcId(topCells[tcTopCells]);
			const size_t tcCells(tcId);
			const Id bcId(mesh->getBottomCell(tcId));
			const size_t bcCells(bcId);
			H_nplus1[tcCells] = H_n[bcCells];
		});
	}
	{
		const auto bottomCells(mesh->getBottomCells());
		const size_t nbBottomCells(bottomCells.size());
		parallel::parallel_exec(nbBottomCells, [&](const size_t& bcBottomCells)
		{
			const Id bcId(bottomCells[bcBottomCells]);
			const size_t bcCells(bcId);
			const Id tcId(mesh->getTopCell(bcId));
			const size_t tcCells(tcId);
			H_nplus1[bcCells] = H_n[tcCells];
		});
	}
	{
		const auto leftCells(mesh->getLeftCells());
		const size_t nbLeftCells(leftCells.size());
		parallel::parallel_exec(nbLeftCells, [&](const size_t& lcLeftCells)
		{
			const Id lcId(leftCells[lcLeftCells]);
			const size_t lcCells(lcId);
			const Id rcId(mesh->getRightCell(lcId));
			const size_t rcCells(rcId);
			H_nplus1[lcCells] = H_n[rcCells];
		});
	}
	{
		const auto rightCells(mesh->getRightCells());
		const size_t nbRightCells(rightCells.size());
		parallel::parallel_exec(nbRightCells, [&](const size_t& rcRightCells)
		{
			const Id rcId(rightCells[rcRightCells]);
			const size_t rcCells(rcId);
			const Id lcId(mesh->getLeftCell(rcId));
			const size_t lcCells(lcId);
			H_nplus1[rcCells] = H_n[lcCells];
		});
	}
}

/**
 * Job UpdateRij called @1.0 in simulate method.
 * In variables: 
 * Out variables: Rij_nplus1
 */
void Swan::updateRij() noexcept
{
	parallel::parallel_exec(nbCells, [&](const size_t& cCells)
	{
		Rij_nplus1[cCells] = 0.0;
	});
}

/**
 * Job UpdateUinner called @1.0 in executeTimeLoopN method.
 * In variables: Bool, C, Dij, F, Fx, H_n, U_n, V_n, deltat, deltax, deltay, g
 * Out variables: U_nplus1
 */
void Swan::updateUinner() noexcept
{
	{
		const auto InnerVerticalFaces(mesh->getInnerVerticalFaces());
		const size_t nbInnerVerticalFaces(InnerVerticalFaces.size());
		parallel::parallel_exec(nbInnerVerticalFaces, [&](const size_t& civfInnerVerticalFaces)
		{
			const Id civfId(InnerVerticalFaces[civfInnerVerticalFaces]);
			const size_t civfFaces(civfId);
			double TV(0.0);
			double TU1(0.0);
			double TU2(0.0);
			double THU(0.0);
			double SB(0.0);
			{
				const Id fijId(mesh->getBottomRightFaceNeighbour(civfId));
				const size_t fijFaces(fijId);
				const Id fijplus1Id(mesh->getTopRightFaceNeighbour(civfId));
				const size_t fijplus1Faces(fijplus1Id);
				const Id fimoins1jplus1Id(mesh->getTopLeftFaceNeighbour(civfId));
				const size_t fimoins1jplus1Faces(fimoins1jplus1Id);
				const Id fimoins1jId(mesh->getBottomLeftFaceNeighbour(civfId));
				const size_t fimoins1jFaces(fimoins1jId);
				TV = 0.25 * (V_n[fijFaces] + V_n[fijplus1Faces] + V_n[fimoins1jplus1Faces] + V_n[fimoins1jFaces]);
			}
			if (U_n[civfFaces] < 0) 
			{
				const Id fiplus1jId(mesh->getRightFaceNeighbour(civfId));
				const size_t fiplus1jFaces(fiplus1jId);
				TU1 = U_n[fiplus1jFaces] - U_n[civfFaces];
			}
			else
			{
				const Id fimoins1jId(mesh->getLeftFaceNeighbour(civfId));
				const size_t fimoins1jFaces(fimoins1jId);
				TU1 = U_n[civfFaces] - U_n[fimoins1jFaces];
			}
			if (TV < 0) 
			{
				const Id fijplus1Id(mesh->getTopFaceNeighbour(civfId));
				const size_t fijplus1Faces(fijplus1Id);
				TU2 = U_n[fijplus1Faces] - U_n[civfFaces];
			}
			else
			{
				const Id fijmoins1Id(mesh->getBottomFaceNeighbour(civfId));
				const size_t fijmoins1Faces(fijmoins1Id);
				TU2 = U_n[civfFaces] - U_n[fijmoins1Faces];
			}
			{
				const Id cijId(mesh->getFrontCell(civfId));
				const size_t cijCells(cijId);
				const Id cimoins1jId(mesh->getLeftCell(cijId));
				const size_t cimoins1jCells(cimoins1jId);
				THU = H_n[cijCells] - H_n[cimoins1jCells];
			}
			{
				const Id cijId(mesh->getFrontCell(civfId));
				const size_t cijCells(cijId);
				const Id fijvId(mesh->getBottomRightFaceNeighbour(civfId));
				const size_t fijvFaces(fijvId);
				SB = g * U_n[civfFaces] * (std::sqrt(U_n[civfFaces] * U_n[civfFaces] + V_n[fijvFaces] * V_n[fijvFaces])) / (C * C * (Dij[cijCells] + H_n[cijCells]));
			}
			{
				const Id fijvId(mesh->getBottomRightFaceNeighbour(civfId));
				const size_t fijvFaces(fijvId);
				const Id cijId(mesh->getFrontCell(civfId));
				const size_t cijCells(cijId);
				const Id cimoins1jId(mesh->getBackCell(civfId));
				const size_t cimoins1jCells(cimoins1jId);
				U_nplus1[civfFaces] = (U_n[civfFaces] - options.deltat * (U_n[civfFaces] * TU1 / deltax + TV * TU2 / deltay) - (g * options.deltat * THU) / deltax + options.deltat * (-options.F * V_n[fijvFaces] - Fx[cijCells] + SB)) * Bool[cijCells] * Bool[cimoins1jCells];
			}
		});
	}
}

/**
 * Job UpdateUouter called @1.0 in executeTimeLoopN method.
 * In variables: U_n
 * Out variables: U_nplus1
 */
void Swan::updateUouter() noexcept
{
	{
		const auto topCells(mesh->getTopCells());
		const size_t nbTopCells(topCells.size());
		parallel::parallel_exec(nbTopCells, [&](const size_t& tcTopCells)
		{
			const Id tcId(topCells[tcTopCells]);
			const Id rfId(mesh->getRightFaceOfCell(tcId));
			const size_t rfFaces(rfId);
			const Id bcId(mesh->getBottomCell(tcId));
			const Id brfId(mesh->getRightFaceOfCell(bcId));
			const size_t brfFaces(brfId);
			U_nplus1[rfFaces] = U_n[brfFaces];
		});
	}
	{
		const auto bottomCells(mesh->getBottomCells());
		const size_t nbBottomCells(bottomCells.size());
		parallel::parallel_exec(nbBottomCells, [&](const size_t& bcBottomCells)
		{
			const Id bcId(bottomCells[bcBottomCells]);
			const Id rfId(mesh->getRightFaceOfCell(bcId));
			const size_t rfFaces(rfId);
			const Id bcfId(mesh->getTopCell(bcId));
			const Id trfId(mesh->getRightFaceOfCell(bcfId));
			const size_t trfFaces(trfId);
			U_nplus1[rfFaces] = U_n[trfFaces];
		});
	}
	{
		const auto leftCells(mesh->getLeftCells());
		const size_t nbLeftCells(leftCells.size());
		parallel::parallel_exec(nbLeftCells, [&](const size_t& lcLeftCells)
		{
			const Id lcId(leftCells[lcLeftCells]);
			const Id lfId(mesh->getLeftFaceOfCell(lcId));
			const size_t lfFaces(lfId);
			const Id rfId(mesh->getRightFaceOfCell(lcId));
			const size_t rfFaces(rfId);
			U_nplus1[lfFaces] = U_n[rfFaces];
		});
	}
	{
		const auto rightCells(mesh->getRightCells());
		const size_t nbRightCells(rightCells.size());
		parallel::parallel_exec(nbRightCells, [&](const size_t& rcRightCells)
		{
			const Id rcId(rightCells[rcRightCells]);
			const Id rfId(mesh->getRightFaceOfCell(rcId));
			const size_t rfFaces(rfId);
			const Id lfId(mesh->getLeftFaceOfCell(rcId));
			const size_t lfFaces(lfId);
			U_nplus1[rfFaces] = U_n[lfFaces];
		});
	}
}

/**
 * Job UpdateVinner called @1.0 in executeTimeLoopN method.
 * In variables: Bool, C, Dij, F, Fy, H_n, U_n, V_n, deltat, deltax, deltay, g
 * Out variables: V_nplus1
 */
void Swan::updateVinner() noexcept
{
	{
		const auto InnerHorizontalFaces(mesh->getInnerHorizontalFaces());
		const size_t nbInnerHorizontalFaces(InnerHorizontalFaces.size());
		parallel::parallel_exec(nbInnerHorizontalFaces, [&](const size_t& cihfInnerHorizontalFaces)
		{
			const Id cihfId(InnerHorizontalFaces[cihfInnerHorizontalFaces]);
			const size_t cihfFaces(cihfId);
			double TU(0.0);
			double TV1(0.0);
			double TV2(0.0);
			double THV(0.0);
			double SB(0.0);
			{
				const Id fijId(mesh->getTopLeftFaceNeighbour(cihfId));
				const size_t fijFaces(fijId);
				const Id fiplus1jId(mesh->getTopRightFaceNeighbour(cihfId));
				const size_t fiplus1jFaces(fiplus1jId);
				const Id fijmoins1Id(mesh->getBottomLeftFaceNeighbour(cihfId));
				const size_t fijmoins1Faces(fijmoins1Id);
				const Id fiplus1jmoins1Id(mesh->getBottomRightFaceNeighbour(cihfId));
				const size_t fiplus1jmoins1Faces(fiplus1jmoins1Id);
				TU = 0.25 * (U_n[fijFaces] + U_n[fiplus1jFaces] + U_n[fijmoins1Faces] + U_n[fiplus1jmoins1Faces]);
			}
			if (TU < 0) 
			{
				const Id fiplus1jId(mesh->getRightFaceNeighbour(cihfId));
				const size_t fiplus1jFaces(fiplus1jId);
				TV1 = V_n[fiplus1jFaces] - V_n[cihfFaces];
			}
			else
			{
				const Id fimoins1jId(mesh->getLeftFaceNeighbour(cihfId));
				const size_t fimoins1jFaces(fimoins1jId);
				TV1 = V_n[cihfFaces] - V_n[fimoins1jFaces];
			}
			if (V_n[cihfFaces] < 0) 
			{
				const Id fijplus1Id(mesh->getTopFaceNeighbour(cihfId));
				const size_t fijplus1Faces(fijplus1Id);
				TV2 = V_n[fijplus1Faces] - V_n[cihfFaces];
			}
			else
			{
				const Id fijmoins1Id(mesh->getBottomFaceNeighbour(cihfId));
				const size_t fijmoins1Faces(fijmoins1Id);
				TV2 = V_n[cihfFaces] - V_n[fijmoins1Faces];
			}
			{
				const Id cijId(mesh->getFrontCell(cihfId));
				const size_t cijCells(cijId);
				const Id cijmoins1Id(mesh->getBackCell(cihfId));
				const size_t cijmoins1Cells(cijmoins1Id);
				THV = H_n[cijCells] - H_n[cijmoins1Cells];
			}
			{
				const Id cijId(mesh->getFrontCell(cihfId));
				const size_t cijCells(cijId);
				const Id fijvId(mesh->getTopLeftFaceNeighbour(cihfId));
				const size_t fijvFaces(fijvId);
				SB = g * U_n[fijvFaces] * (std::sqrt(U_n[fijvFaces] * U_n[fijvFaces] + V_n[cihfFaces] * V_n[cihfFaces])) / (C * C * (Dij[cijCells] + H_n[cijCells]));
			}
			{
				const Id fijvId(mesh->getTopLeftFaceNeighbour(cihfId));
				const size_t fijvFaces(fijvId);
				const Id cijId(mesh->getFrontCell(cihfId));
				const size_t cijCells(cijId);
				const Id cijmoins1Id(mesh->getBackCell(cihfId));
				const size_t cijmoins1Cells(cijmoins1Id);
				V_nplus1[cihfFaces] = (V_n[cihfFaces] - options.deltat * (TU * TV1 / deltax + V_n[cihfFaces] * TV2 / deltay) - (g * options.deltat * THV) / deltay + options.deltat * (options.F * U_n[fijvFaces] - Fy[cijCells] + SB)) * Bool[cijmoins1Cells] * Bool[cijCells];
			}
		});
	}
}

/**
 * Job UpdateVouter called @1.0 in executeTimeLoopN method.
 * In variables: V_n
 * Out variables: V_nplus1
 */
void Swan::updateVouter() noexcept
{
	{
		const auto topCells(mesh->getTopCells());
		const size_t nbTopCells(topCells.size());
		parallel::parallel_exec(nbTopCells, [&](const size_t& tcTopCells)
		{
			const Id tcId(topCells[tcTopCells]);
			const Id bfId(mesh->getBottomFaceOfCell(tcId));
			const size_t bfFaces(bfId);
			const Id tfId(mesh->getTopFaceOfCell(tcId));
			const size_t tfFaces(tfId);
			V_nplus1[tfFaces] = V_n[bfFaces];
		});
	}
	{
		const auto bottomCells(mesh->getBottomCells());
		const size_t nbBottomCells(bottomCells.size());
		parallel::parallel_exec(nbBottomCells, [&](const size_t& bcBottomCells)
		{
			const Id bcId(bottomCells[bcBottomCells]);
			const Id bfId(mesh->getBottomFaceOfCell(bcId));
			const size_t bfFaces(bfId);
			const Id tfId(mesh->getTopFaceOfCell(bcId));
			const size_t tfFaces(tfId);
			V_nplus1[bfFaces] = V_n[tfFaces];
		});
	}
	{
		const auto leftCells(mesh->getLeftCells());
		const size_t nbLeftCells(leftCells.size());
		parallel::parallel_exec(nbLeftCells, [&](const size_t& lcLeftCells)
		{
			const Id lcId(leftCells[lcLeftCells]);
			const Id bfId(mesh->getBottomFaceOfCell(lcId));
			const size_t bfFaces(bfId);
			const Id rcId(mesh->getRightCell(lcId));
			const Id bfrcId(mesh->getBottomFaceOfCell(rcId));
			const size_t bfrcFaces(bfrcId);
			V_nplus1[bfFaces] = V_n[bfrcFaces];
		});
	}
	{
		const auto rightCells(mesh->getRightCells());
		const size_t nbRightCells(rightCells.size());
		parallel::parallel_exec(nbRightCells, [&](const size_t& rcRightCells)
		{
			const Id rcId(rightCells[rcRightCells]);
			const Id bfId(mesh->getBottomFaceOfCell(rcId));
			const size_t bfFaces(bfId);
			const Id lcId(mesh->getLeftCell(rcId));
			const Id bflcId(mesh->getBottomFaceOfCell(lcId));
			const size_t bflcFaces(bflcId);
			V_nplus1[bfFaces] = V_n[bflcFaces];
		});
	}
}

/**
 * Job InitDij called @2.0 in simulate method.
 * In variables: Dijini
 * Out variables: Dij
 */
void Swan::initDij() noexcept
{
	{
		const auto topCells(mesh->getTopCells());
		const size_t nbTopCells(topCells.size());
		parallel::parallel_exec(nbTopCells, [&](const size_t& tcTopCells)
		{
			const Id tcId(topCells[tcTopCells]);
			const size_t tcCells(tcId);
			const Id bcId(mesh->getBottomCell(tcId));
			const size_t bcCells(bcId);
			Dij[tcCells] = Dijini[bcCells];
		});
	}
	{
		const auto bottomCells(mesh->getBottomCells());
		const size_t nbBottomCells(bottomCells.size());
		parallel::parallel_exec(nbBottomCells, [&](const size_t& bcBottomCells)
		{
			const Id bcId(bottomCells[bcBottomCells]);
			const size_t bcCells(bcId);
			const Id tcId(mesh->getTopCell(bcId));
			const size_t tcCells(tcId);
			Dij[bcCells] = Dijini[tcCells];
		});
	}
	{
		const auto leftCells(mesh->getLeftCells());
		const size_t nbLeftCells(leftCells.size());
		parallel::parallel_exec(nbLeftCells, [&](const size_t& lcLeftCells)
		{
			const Id lcId(leftCells[lcLeftCells]);
			const size_t lcCells(lcId);
			const Id rcId(mesh->getRightCell(lcId));
			const size_t rcCells(rcId);
			Dij[lcCells] = Dijini[rcCells];
		});
	}
	{
		const auto rightCells(mesh->getRightCells());
		const size_t nbRightCells(rightCells.size());
		parallel::parallel_exec(nbRightCells, [&](const size_t& rcRightCells)
		{
			const Id rcId(rightCells[rcRightCells]);
			const size_t rcCells(rcId);
			const Id lcId(mesh->getLeftCell(rcId));
			const size_t lcCells(lcId);
			Dij[rcCells] = Dijini[lcCells];
		});
	}
	{
		const auto innerCells(mesh->getInnerCells());
		const size_t nbInnerCells(innerCells.size());
		parallel::parallel_exec(nbInnerCells, [&](const size_t& icInnerCells)
		{
			const Id icId(innerCells[icInnerCells]);
			const size_t icCells(icId);
			Dij[icCells] = Dijini[icCells];
		});
	}
}

/**
 * Job InitRij called @2.0 in simulate method.
 * In variables: Rijini
 * Out variables: Rij_n0
 */
void Swan::initRij() noexcept
{
	{
		const auto topCells(mesh->getTopCells());
		const size_t nbTopCells(topCells.size());
		parallel::parallel_exec(nbTopCells, [&](const size_t& tcTopCells)
		{
			const Id tcId(topCells[tcTopCells]);
			const size_t tcCells(tcId);
			const Id bcId(mesh->getBottomCell(tcId));
			const size_t bcCells(bcId);
			Rij_n0[tcCells] = Rijini[bcCells];
		});
	}
	{
		const auto bottomCells(mesh->getBottomCells());
		const size_t nbBottomCells(bottomCells.size());
		parallel::parallel_exec(nbBottomCells, [&](const size_t& bcBottomCells)
		{
			const Id bcId(bottomCells[bcBottomCells]);
			const size_t bcCells(bcId);
			const Id tcId(mesh->getTopCell(bcId));
			const size_t tcCells(tcId);
			Rij_n0[bcCells] = Rijini[tcCells];
		});
	}
	{
		const auto leftCells(mesh->getLeftCells());
		const size_t nbLeftCells(leftCells.size());
		parallel::parallel_exec(nbLeftCells, [&](const size_t& lcLeftCells)
		{
			const Id lcId(leftCells[lcLeftCells]);
			const size_t lcCells(lcId);
			const Id rcId(mesh->getRightCell(lcId));
			const size_t rcCells(rcId);
			Rij_n0[lcCells] = Rijini[rcCells];
		});
	}
	{
		const auto rightCells(mesh->getRightCells());
		const size_t nbRightCells(rightCells.size());
		parallel::parallel_exec(nbRightCells, [&](const size_t& rcRightCells)
		{
			const Id rcId(rightCells[rcRightCells]);
			const size_t rcCells(rcId);
			const Id lcId(mesh->getLeftCell(rcId));
			const size_t lcCells(lcId);
			Rij_n0[rcCells] = Rijini[lcCells];
		});
	}
	{
		const auto innerCells(mesh->getInnerCells());
		const size_t nbInnerCells(innerCells.size());
		parallel::parallel_exec(nbInnerCells, [&](const size_t& icInnerCells)
		{
			const Id icId(innerCells[icInnerCells]);
			const size_t icCells(icId);
			Rij_n0[icCells] = Rijini[icCells];
		});
	}
}

/**
 * Job InitU called @2.0 in simulate method.
 * In variables: Uini
 * Out variables: U_n0
 */
void Swan::initU() noexcept
{
	{
		const auto topCells(mesh->getTopCells());
		const size_t nbTopCells(topCells.size());
		parallel::parallel_exec(nbTopCells, [&](const size_t& tcTopCells)
		{
			const Id tcId(topCells[tcTopCells]);
			const Id rfId(mesh->getRightFaceOfCell(tcId));
			const size_t rfFaces(rfId);
			const Id bcId(mesh->getBottomCell(tcId));
			const Id brfId(mesh->getRightFaceOfCell(bcId));
			const size_t brfFaces(brfId);
			U_n0[rfFaces] = Uini[brfFaces];
		});
	}
	{
		const auto bottomCells(mesh->getBottomCells());
		const size_t nbBottomCells(bottomCells.size());
		parallel::parallel_exec(nbBottomCells, [&](const size_t& bcBottomCells)
		{
			const Id bcId(bottomCells[bcBottomCells]);
			const Id rfId(mesh->getRightFaceOfCell(bcId));
			const size_t rfFaces(rfId);
			const Id tcId(mesh->getTopCell(bcId));
			const Id trfId(mesh->getRightFaceOfCell(tcId));
			const size_t trfFaces(trfId);
			U_n0[rfFaces] = Uini[trfFaces];
		});
	}
	{
		const auto leftCells(mesh->getLeftCells());
		const size_t nbLeftCells(leftCells.size());
		parallel::parallel_exec(nbLeftCells, [&](const size_t& lcLeftCells)
		{
			const Id lcId(leftCells[lcLeftCells]);
			const Id lfId(mesh->getLeftFaceOfCell(lcId));
			const size_t lfFaces(lfId);
			const Id rfId(mesh->getRightFaceOfCell(lcId));
			const size_t rfFaces(rfId);
			U_n0[lfFaces] = Uini[rfFaces];
		});
	}
	{
		const auto rightCells(mesh->getRightCells());
		const size_t nbRightCells(rightCells.size());
		parallel::parallel_exec(nbRightCells, [&](const size_t& rcRightCells)
		{
			const Id rcId(rightCells[rcRightCells]);
			const Id rfId(mesh->getRightFaceOfCell(rcId));
			const size_t rfFaces(rfId);
			const Id lfId(mesh->getLeftFaceOfCell(rcId));
			const size_t lfFaces(lfId);
			U_n0[rfFaces] = Uini[lfFaces];
		});
	}
}

/**
 * Job InitV called @2.0 in simulate method.
 * In variables: Vini
 * Out variables: V_n0
 */
void Swan::initV() noexcept
{
	{
		const auto topCells(mesh->getTopCells());
		const size_t nbTopCells(topCells.size());
		parallel::parallel_exec(nbTopCells, [&](const size_t& tcTopCells)
		{
			const Id tcId(topCells[tcTopCells]);
			const Id bfId(mesh->getBottomFaceOfCell(tcId));
			const size_t bfFaces(bfId);
			const Id tfId(mesh->getTopFaceOfCell(tcId));
			const size_t tfFaces(tfId);
			V_n0[tfFaces] = Vini[bfFaces];
		});
	}
	{
		const auto bottomCells(mesh->getBottomCells());
		const size_t nbBottomCells(bottomCells.size());
		parallel::parallel_exec(nbBottomCells, [&](const size_t& bcBottomCells)
		{
			const Id bcId(bottomCells[bcBottomCells]);
			const Id bfId(mesh->getBottomFaceOfCell(bcId));
			const size_t bfFaces(bfId);
			const Id tfId(mesh->getTopFaceOfCell(bcId));
			const size_t tfFaces(tfId);
			V_n0[bfFaces] = Vini[tfFaces];
		});
	}
	{
		const auto leftCells(mesh->getLeftCells());
		const size_t nbLeftCells(leftCells.size());
		parallel::parallel_exec(nbLeftCells, [&](const size_t& lcLeftCells)
		{
			const Id lcId(leftCells[lcLeftCells]);
			const Id bfId(mesh->getBottomFaceOfCell(lcId));
			const size_t bfFaces(bfId);
			const Id rcId(mesh->getRightCell(lcId));
			const Id bfrcId(mesh->getBottomFaceOfCell(rcId));
			const size_t bfrcFaces(bfrcId);
			V_n0[bfFaces] = Vini[bfrcFaces];
		});
	}
	{
		const auto rightCells(mesh->getRightCells());
		const size_t nbRightCells(rightCells.size());
		parallel::parallel_exec(nbRightCells, [&](const size_t& rcRightCells)
		{
			const Id rcId(rightCells[rcRightCells]);
			const Id bfId(mesh->getBottomFaceOfCell(rcId));
			const size_t bfFaces(bfId);
			const Id lcId(mesh->getLeftCell(rcId));
			const Id bflcId(mesh->getBottomFaceOfCell(lcId));
			const size_t bflcFaces(bflcId);
			V_n0[bfFaces] = Vini[bflcFaces];
		});
	}
}

/**
 * Job InitXcAndYc called @2.0 in simulate method.
 * In variables: Xc
 * Out variables: xc, yc
 */
void Swan::initXcAndYc() noexcept
{
	parallel::parallel_exec(nbCells, [&](const size_t& cCells)
	{
		xc[cCells] = Xc[cCells][0];
		yc[cCells] = Xc[cCells][1];
	});
}

/**
 * Job InitBool called @3.0 in simulate method.
 * In variables: Dij
 * Out variables: Bool
 */
void Swan::initBool() noexcept
{
	parallel::parallel_exec(nbCells, [&](const size_t& cCells)
	{
		if (Dij[cCells] >= 0.0) 
			Bool[cCells] = 0.0;
		else
			Bool[cCells] = 1.0;
	});
}

/**
 * Job InitHini called @4.0 in simulate method.
 * In variables: Bool
 * Out variables: Hini
 */
void Swan::initHini() noexcept
{
	{
		const auto innerCells(mesh->getInnerCells());
		const size_t nbInnerCells(innerCells.size());
		for (size_t icInnerCells=0; icInnerCells<nbInnerCells; icInnerCells++)
		{
			const Id icId(innerCells[icInnerCells]);
			const size_t icCells(icId);
			Hini[icCells] = swanFunctions.nextWaveHeight() * Bool[icCells];
		}
	}
}

/**
 * Job InitH called @5.0 in simulate method.
 * In variables: Hini
 * Out variables: H_n0
 */
void Swan::initH() noexcept
{
	{
		const auto topCells(mesh->getTopCells());
		const size_t nbTopCells(topCells.size());
		parallel::parallel_exec(nbTopCells, [&](const size_t& tTopCells)
		{
			const Id tId(topCells[tTopCells]);
			const size_t tCells(tId);
			const Id btId(mesh->getBottomCell(tId));
			const size_t btCells(btId);
			H_n0[tCells] = Hini[btCells];
		});
	}
	{
		const auto bottomCells(mesh->getBottomCells());
		const size_t nbBottomCells(bottomCells.size());
		parallel::parallel_exec(nbBottomCells, [&](const size_t& bBottomCells)
		{
			const Id bId(bottomCells[bBottomCells]);
			const size_t bCells(bId);
			const Id tbId(mesh->getTopCell(bId));
			const size_t tbCells(tbId);
			H_n0[bCells] = Hini[tbCells];
		});
	}
	{
		const auto leftCells(mesh->getLeftCells());
		const size_t nbLeftCells(leftCells.size());
		parallel::parallel_exec(nbLeftCells, [&](const size_t& lLeftCells)
		{
			const Id lId(leftCells[lLeftCells]);
			const size_t lCells(lId);
			const Id rlId(mesh->getRightCell(lId));
			const size_t rlCells(rlId);
			H_n0[lCells] = Hini[rlCells];
		});
	}
	{
		const auto rightCells(mesh->getRightCells());
		const size_t nbRightCells(rightCells.size());
		parallel::parallel_exec(nbRightCells, [&](const size_t& rRightCells)
		{
			const Id rId(rightCells[rRightCells]);
			const size_t rCells(rId);
			const Id lrId(mesh->getLeftCell(rId));
			const size_t lrCells(lrId);
			H_n0[rCells] = Hini[lrCells];
		});
	}
	{
		const auto innerCells(mesh->getInnerCells());
		const size_t nbInnerCells(innerCells.size());
		parallel::parallel_exec(nbInnerCells, [&](const size_t& icInnerCells)
		{
			const Id icId(innerCells[icInnerCells]);
			const size_t icCells(icId);
			H_n0[icCells] = Hini[icCells];
		});
	}
}

/**
 * Job SetUpTimeLoopN called @6.0 in simulate method.
 * In variables: H_n0, Rij_n0, U_n0, V_n0
 * Out variables: H_n, Rij_n, U_n, V_n
 */
void Swan::setUpTimeLoopN() noexcept
{
	for (size_t i1(0) ; i1<U_n.size() ; i1++)
		U_n[i1] = U_n0[i1];
	for (size_t i1(0) ; i1<V_n.size() ; i1++)
		V_n[i1] = V_n0[i1];
	for (size_t i1(0) ; i1<H_n.size() ; i1++)
		H_n[i1] = H_n0[i1];
	for (size_t i1(0) ; i1<Rij_n.size() ; i1++)
		Rij_n[i1] = Rij_n0[i1];
}

/**
 * Job ExecuteTimeLoopN called @7.0 in simulate method.
 * In variables: Bool, C, Dij, F, Fx, Fy, H_n, Rij_n, Rij_nplus1, U_n, V_n, deltat, deltax, deltay, g, t_n
 * Out variables: H_nplus1, U_nplus1, V_nplus1, t_nplus1
 */
void Swan::executeTimeLoopN() noexcept
{
	n = 0;
	bool continueLoop = true;
	do
	{
		globalTimer.start();
		cpuTimer.start();
		n++;
		if (!writer.isDisabled() && n >= lastDump + options.outputPeriod)
			dumpVariables(n);
		if (n!=1)
			std::cout << "[" << __CYAN__ << __BOLD__ << setw(3) << n << __RESET__ "] t = " << __BOLD__
				<< setiosflags(std::ios::scientific) << setprecision(8) << setw(16) << t_n << __RESET__;
	
		computeTn(); // @1.0
		updateHinner(); // @1.0
		updateHouter(); // @1.0
		updateUinner(); // @1.0
		updateUouter(); // @1.0
		updateVinner(); // @1.0
		updateVouter(); // @1.0
		
	
		// Evaluate loop condition with variables at time n
		continueLoop = (t_nplus1 < options.stopTime && n < options.maxIter);
	
		if (continueLoop)
		{
			// Switch variables to prepare next iteration
			std::swap(t_nplus1, t_n);
			std::swap(U_nplus1, U_n);
			std::swap(V_nplus1, V_n);
			std::swap(H_nplus1, H_n);
			std::swap(Rij_nplus1, Rij_n);
		}
	
		cpuTimer.stop();
		globalTimer.stop();
	
		// Timers display
		if (!writer.isDisabled())
			std::cout << " {CPU: " << __BLUE__ << cpuTimer.print(true) << __RESET__ ", IO: " << __BLUE__ << ioTimer.print(true) << __RESET__ "} ";
		else
			std::cout << " {CPU: " << __BLUE__ << cpuTimer.print(true) << __RESET__ ", IO: " << __RED__ << "none" << __RESET__ << "} ";
		
		// Progress
		std::cout << utils::progress_bar(n, options.maxIter, t_n, options.stopTime, 25);
		std::cout << __BOLD__ << __CYAN__ << utils::Timer::print(
			utils::eta(n, options.maxIter, t_n, options.stopTime, options.deltat, globalTimer), true)
			<< __RESET__ << "\r";
		std::cout.flush();
	
		cpuTimer.reset();
		ioTimer.reset();
	} while (continueLoop);
	// force a last output at the end
	dumpVariables(n, false);
}

void Swan::dumpVariables(int iteration, bool useTimer)
{
	if (!writer.isDisabled())
	{
		if (useTimer)
		{
			cpuTimer.stop();
			ioTimer.start();
		}
		auto quads = mesh->getGeometry()->getQuads();
		writer.startVtpFile(iteration, t_n, nbNodes, X.data(), nbCells, quads.data());
		writer.openNodeData();
		writer.closeNodeData();
		writer.openCellData();
		writer.write("bottommotion", Rij_n);
		writer.write("profondeur", Dij);
		writer.write("hauteur", H_n);
		writer.closeCellData();
		writer.closeVtpFile();
		lastDump = n;
		if (useTimer)
		{
			ioTimer.stop();
			cpuTimer.start();
		}
	}
}

void Swan::simulate()
{
	std::cout << "\n" << __BLUE_BKG__ << __YELLOW__ << __BOLD__ <<"\tStarting Swan ..." << __RESET__ << "\n\n";
	
	std::cout << "[" << __GREEN__ << "MESH" << __RESET__ << "]      X=" << __BOLD__ << options.X_EDGE_ELEMS << __RESET__ << ", Y=" << __BOLD__ << options.Y_EDGE_ELEMS
		<< __RESET__ << ", X length=" << __BOLD__ << options.X_EDGE_LENGTH << __RESET__ << ", Y length=" << __BOLD__ << options.Y_EDGE_LENGTH << __RESET__ << std::endl;
	
	std::cout << "[" << __GREEN__ << "TOPOLOGY" << __RESET__ << "]  HWLOC unavailable cannot get topological informations" << std::endl;
	
	if (!writer.isDisabled())
		std::cout << "[" << __GREEN__ << "OUTPUT" << __RESET__ << "]    VTK files stored in " << __BOLD__ << writer.outputDirectory() << __RESET__ << " directory" << std::endl;
	else
		std::cout << "[" << __GREEN__ << "OUTPUT" << __RESET__ << "]    " << __BOLD__ << "Disabled" << __RESET__ << std::endl;

	initDijini(); // @1.0
	initFxy(); // @1.0
	initRijini(); // @1.0
	initUini(); // @1.0
	initVini(); // @1.0
	initXc(); // @1.0
	updateRij(); // @1.0
	initDij(); // @2.0
	initRij(); // @2.0
	initU(); // @2.0
	initV(); // @2.0
	initXcAndYc(); // @2.0
	initBool(); // @3.0
	initHini(); // @4.0
	initH(); // @5.0
	setUpTimeLoopN(); // @6.0
	executeTimeLoopN(); // @7.0
	
	std::cout << __YELLOW__ << "\n\tDone ! Took " << __MAGENTA__ << __BOLD__ << globalTimer.print() << __RESET__ << std::endl;
}

/******************** Module definition ********************/

int main(int argc, char* argv[]) 
{
	string dataFile;
	
	if (argc == 2)
	{
		dataFile = argv[1];
	}
	else
	{
		std::cerr << "[ERROR] Wrong number of arguments. Expecting 1 arg: dataFile." << std::endl;
		std::cerr << "(SwanDefaultOptions.json)" << std::endl;
		return -1;
	}
	
	// read json dataFile
	ifstream ifs(dataFile);
	rapidjson::IStreamWrapper isw(ifs);
	rapidjson::Document d;
	d.ParseStream(isw);
	assert(d.IsObject());
	
	// options
	Swan::Options options;
	if (d.HasMember("options"))
	{
		const rapidjson::Value& valueof_options = d["options"];
		assert(valueof_options.IsObject());
		options.jsonInit(valueof_options.GetObject());
	}
	
	// swanFunctions
	SwanFunctions swanFunctions;
	if (d.HasMember("swanFunctions"))
	{
		const rapidjson::Value& valueof_swanFunctions = d["swanFunctions"];
		assert(valueof_swanFunctions.IsObject());
		swanFunctions.jsonInit(valueof_swanFunctions.GetObject());
	}
	
	// simulator must be a pointer if there is a finalize at the end (Kokkos, omp...)
	auto simulator = new Swan(options, swanFunctions);
	simulator->simulate();
	
	// simulator must be deleted before calling finalize
	delete simulator;
	return 0;
}
