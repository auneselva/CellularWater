// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldController.h"
#include "WaterCube.h"
#include "Grid3d.h"
#include <vector>
#include <algorithm>
#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>
#include "ActorSpawner.h"
#include <WorldBorder.h>

AWorldController::AWorldController()
{
	waterSimGameInstance = Cast<UWaterSimGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	PrimaryActorTick.bCanEverTick = true;
	defaultRotation = new FRotator3d(0,0,0);
	Grid3d::GetInstance();

}
void AWorldController::BeginPlay()
{
	Super::BeginPlay();

	gameTimeElapsed = 0;
	simCounter = 0;
}
AWorldController::~AWorldController()
{
	delete defaultRotation;
}

void AWorldController::Tick(float DeltaTime) //delta time == around 0.02
{
	Super::Tick(DeltaTime);
	gameTimeElapsed += DeltaTime;
	if (gameTimeElapsed > SIMULATION_SPEED) {
		gameTimeElapsed -= SIMULATION_SPEED;
		ApplySimulationProccesses();
		simCounter++;
	}
		
}

void AWorldController::Gravity(const int& index) {

	if (CanWaterFallDown(index)) {
		int bottomIndex = Grid3d::GetBottomNeighborIndex(index);
		if (Grid3d::GetInstance()->GetWaterCubeIfVisible(bottomIndex) != nullptr) {
			float waterAmountToBeFlown = Grid3d::GetInstance()->GetWaterCapacity(bottomIndex) - Grid3d::GetInstance()->GetCurrentWaterLevel(bottomIndex) - Grid3d::GetInstance()->GetWaterSpilt(bottomIndex);
			
			Grid3d::GetInstance()->AddWaterSpilt(bottomIndex, waterAmountToBeFlown);
			Grid3d::GetInstance()->AddWaterSpilt(index, -waterAmountToBeFlown);
		}
		else {
			Grid3d::GetInstance()->AddWaterSpilt(bottomIndex, Grid3d::GetInstance()->GetCurrentWaterLevel(index) + Grid3d::GetInstance()->GetWaterSpilt(index));
			Grid3d::GetInstance()->AddWaterSpilt(index, -Grid3d::GetInstance()->GetCurrentWaterLevel(index) - Grid3d::GetInstance()->GetWaterSpilt(index));
		}
	}
}

void AWorldController::ApplySimulationProccesses() {
	
	for (int i = 0; i < N_CELLS; i++) {
		if (Grid3d::GetInstance()->GetWaterCubeIfVisible(i) != nullptr) {
			Gravity(i);
			SpillAround(i);
		}
		if (i % XYNCELLS == XYNCELLS - 1)
			HandleSpiltWater();
	}
	/*
	UE_LOG(LogTemp, Warning, TEXT("\n\n\n\n\nWater Splitted"));
	for (int i = 0; i < N_CELLS; i++) {
		if (GetWaterSpilt(i) > PRECISION_OFFSET)
			UE_LOG(LogTemp, Warning, TEXT("%d: %f"), i, GetWaterSpilt(i));
	}

	UE_LOG(LogTemp, Warning, TEXT("\n\n\n\n\nWater Levels"));
	for (int i = 0; i < N_CELLS; i++) {
		if (GetCurrentWaterLevel(i) > PRECISION_OFFSET)
			UE_LOG(LogTemp, Warning, TEXT("%d: %f"), i, GetCurrentWaterLevel(i))
	} */
	HandleSpiltWater();
	
	CalculateWaterCubeCapacity();
	ApplyCalculatedCapacities(); //
	ClusterizeWaterGroupsOnLevels();
	for (int i = 0; i < N_CELLS; i++)
		if (Grid3d::GetInstance()->GetWaterCubeIfVisible(i) != nullptr)
			UE_LOG(LogTemp, Warning, TEXT("GetCurrentWaterLevel(%d): %f, GetWaterSpilt() %f, GetWaterCapacity(): %f"), i, Grid3d::GetInstance()->GetCurrentWaterLevel(i), Grid3d::GetInstance()->GetWaterSpilt(i), Grid3d::GetInstance()->GetWaterCapacity(i));
	
	DetermineWaterFlow();
	FlowPressurizedWaterUpwards();
	UE_LOG(LogTemp, Warning, TEXT("After Pressurizing"));
	for (int i = 0; i < N_CELLS; i++)
		if (Grid3d::GetInstance()->GetWaterCubeIfVisible(i) != nullptr)
			UE_LOG(LogTemp, Warning, TEXT("GetCurrentWaterLevel(%d): %f, GetWaterSpilt() %f, GetWaterCapacity(): %f"), i, Grid3d::GetInstance()->GetCurrentWaterLevel(i), Grid3d::GetInstance()->GetWaterSpilt(i), Grid3d::GetInstance()->GetWaterCapacity(i));

	HandleSpiltWater();
	Grid3d::GetInstance()->UpdateCubesTransform();
}

void AWorldController::SpillAround(const int& index) {
	//only spill water around if the cell below is occupied

	if (!CanWaterFallDown(index)) {
		//UE_LOG(LogTemp, Warning, TEXT("Cannot fall down"));
		std::vector<int> sideNeighbours;
		sideNeighbours.reserve(4);
		std::vector<int> diagonalNeighbours;
		diagonalNeighbours.reserve(4);
		//UE_LOG(LogTemp, Warning, TEXT("Current index: %d"), index);
		int rightIndex = Grid3d::GetRightNeighborIndex(index);

		if (IsNeighbourFreeToBeSpilledTo(index, rightIndex)) {
			sideNeighbours.emplace_back(rightIndex);
		}
		int leftIndex = Grid3d::GetLeftNeighborIndex(index);
		if (IsNeighbourFreeToBeSpilledTo(index, leftIndex)) {
			sideNeighbours.emplace_back(leftIndex);
		}
		int frontIndex = Grid3d::GetFrontNeighborIndex(index);
		if (IsNeighbourFreeToBeSpilledTo(index, frontIndex)) {
			sideNeighbours.emplace_back(frontIndex);
		}
		int behindIndex = Grid3d::GetBehindNeighborIndex(index);
		if (IsNeighbourFreeToBeSpilledTo(index, behindIndex)) {
			sideNeighbours.emplace_back(behindIndex);
		}

		if (std::find(sideNeighbours.begin(), sideNeighbours.end(), frontIndex) != sideNeighbours.end()) {
			if (std::find(sideNeighbours.begin(), sideNeighbours.end(), rightIndex) != sideNeighbours.end()) {
				int frontRightIndex = Grid3d::GetRightNeighborIndex(frontIndex);
				if (IsNeighbourFreeToBeSpilledTo(index, frontRightIndex))
					diagonalNeighbours.emplace_back(frontRightIndex);
			}	
		}

		if (std::find(sideNeighbours.begin(), sideNeighbours.end(), rightIndex) != sideNeighbours.end()) {
			if (std::find(sideNeighbours.begin(), sideNeighbours.end(), behindIndex) != sideNeighbours.end()) {
				int rightBehindIndex = Grid3d::GetBehindNeighborIndex(rightIndex);
				if (IsNeighbourFreeToBeSpilledTo(index, rightBehindIndex))
					diagonalNeighbours.emplace_back(rightBehindIndex);
			}
		}

		if (std::find(sideNeighbours.begin(), sideNeighbours.end(), behindIndex) != sideNeighbours.end()) {
			if (std::find(sideNeighbours.begin(), sideNeighbours.end(), leftIndex) != sideNeighbours.end()) {
				int behindLeftIndex = Grid3d::GetLeftNeighborIndex(behindIndex);
				if (IsNeighbourFreeToBeSpilledTo(index, behindLeftIndex))
					diagonalNeighbours.emplace_back(behindLeftIndex);
			}
		}

		if (std::find(sideNeighbours.begin(), sideNeighbours.end(), rightIndex) != sideNeighbours.end()) {
			if (std::find(sideNeighbours.begin(), sideNeighbours.end(), frontIndex) != sideNeighbours.end()) {
				int frontRightIndex = Grid3d::GetFrontNeighborIndex(rightIndex);
				if (IsNeighbourFreeToBeSpilledTo(index, frontRightIndex))
					diagonalNeighbours.emplace_back(frontRightIndex);
			}
		}
		/*
		UE_LOG(LogTemp, Warning, TEXT("frontIndex: %d"), frontIndex);
		UE_LOG(LogTemp, Warning, TEXT("behindIndex: %d"), behindIndex);
		UE_LOG(LogTemp, Warning, TEXT("leftIndex: %d"), leftIndex);
		UE_LOG(LogTemp, Warning, TEXT("rightIndex: %d"), rightIndex);
		*/
		// weightSideNeighbour = 1.0f;
		// weightDiagNeighbour = 0.5f;
		// CurrentLevel = (sideNeighbours.size() + 1) * 1.0f * x + diagNeighbours.size() * 0.5f * x
		// CurrentLevel = x ( (sideneighbours.size() + 1) + diagNeighbours.size * 0.5)
		// x = CurrentLevel/ ( (sideneighbours.size() + 1) + diagNeighbours.size * 0.5)
		float diagWeight = 0.5f;

		float diagWaterSum = 0.0f;
		float sideWaterSum = 0.0f;
		for (int& i : sideNeighbours)
			sideWaterSum += Grid3d::GetInstance()->GetCurrentWaterLevel(i) + Grid3d::GetInstance()->GetWaterSpilt(i);
		for (int& i : diagonalNeighbours)
			diagWaterSum += Grid3d::GetInstance()->GetCurrentWaterLevel(i) + Grid3d::GetInstance()->GetWaterSpilt(i);
		float sumWater = sideWaterSum + diagWaterSum + Grid3d::GetInstance()->GetCurrentWaterLevel(index) + Grid3d::GetInstance()->GetWaterSpilt(index);
		const int cells = sideNeighbours.size() + diagonalNeighbours.size() + 1;
		float waterAmountForEach = sumWater / cells;
		//UE_LOG(LogTemp, Warning, TEXT("waterAmountForEachNeighbour: %f"), waterAmountForEachNeighbour);
		for (int& i : sideNeighbours)
			Grid3d::GetInstance()->AddWaterSpilt(i, waterAmountForEach - (Grid3d::GetInstance()->GetCurrentWaterLevel(i) + Grid3d::GetInstance()->GetWaterSpilt(i)));
		for (int& i : diagonalNeighbours)
			Grid3d::GetInstance()->AddWaterSpilt(i, waterAmountForEach - (Grid3d::GetInstance()->GetCurrentWaterLevel(i) + Grid3d::GetInstance()->GetWaterSpilt(i)));
		Grid3d::GetInstance()->AddWaterSpilt(index, waterAmountForEach - (Grid3d::GetInstance()->GetCurrentWaterLevel(index) + Grid3d::GetInstance()->GetWaterSpilt(index)));
	}
}
bool AWorldController::IsNeighbourFreeToBeSpilledTo(const int& currentIndex, const int& neighbourIndex) {
	if (Grid3d::GetInstance()->CheckIfCellWIthinBounds(neighbourIndex)) {
		if (Grid3d::GetInstance()->CheckIfBlockCell(neighbourIndex))
			return false;
		if (Grid3d::GetInstance()->GetWaterCubeIfVisible(neighbourIndex) == nullptr)
			return true;
		if (Grid3d::GetInstance()->GetCurrentWaterLevel(neighbourIndex) + Grid3d::GetInstance()->GetWaterSpilt(neighbourIndex) < Grid3d::GetInstance()->GetCurrentWaterLevel(currentIndex) + Grid3d::GetInstance()->GetWaterSpilt(currentIndex))
			return true;
	}
	return false;
}
bool AWorldController::CanWaterFallDown(const int& currentIndex) {
	int bottomIndex = Grid3d::GetBottomNeighborIndex(currentIndex);
	if (Grid3d::GetInstance()->CheckIfCellWIthinBounds(bottomIndex)) {
		if (Grid3d::GetInstance()->CheckIfBlockCell(bottomIndex))
			return false;
		if (Grid3d::GetInstance()->GetWaterCubeIfVisible(bottomIndex) == nullptr)
			return true;
		if (!Grid3d::GetInstance()->CheckIfFullCapacityReached(bottomIndex, Grid3d::GetInstance()->GetWaterSpilt(bottomIndex) + Grid3d::GetInstance()->GetCurrentWaterLevel(bottomIndex)))
			return true;
	}
	return false;
}

bool AWorldController::CanWaterSpillAround(const int& index) {
	int bottomIndex = Grid3d::GetBottomNeighborIndex(index);
	if (!Grid3d::GetInstance()->CheckIfCellWIthinBounds(bottomIndex))
		return true;
	if (Grid3d::GetInstance()->CheckIfBlockCell(bottomIndex))
		return true;
	if (Grid3d::GetInstance()->GetWaterCubeIfVisible(bottomIndex) == nullptr)
		return true;
	if (!Grid3d::GetInstance()->CheckIfFullCapacityReached(bottomIndex, Grid3d::GetInstance()->GetCurrentWaterLevel(bottomIndex)))
		return true;
	if ( 0.2f > Grid3d::GetInstance()->GetWaterSpilt(bottomIndex)  && Grid3d::GetInstance()->GetWaterSpilt(bottomIndex) > PRECISION_OFFSET)
		return true;
	return false;

}

void AWorldController::HandleSpiltWater() { //or handle pressure?
	for (int i = 0; i < N_CELLS; i++) {
		if (!Grid3d::GetInstance()->CheckIfBlockCell(i)) {
			float summedWaterInCell = Grid3d::GetInstance()->GetWaterSpilt(i) + Grid3d::GetInstance()->GetCurrentWaterLevel(i);
			if (summedWaterInCell < PRECISION_OFFSET) {
				Grid3d::GetInstance()->SetWaterCubeVisibility(i, false);
				Grid3d::GetInstance()->SetNextIterationWaterLevel(i, 0.0f);
			}
			else if (summedWaterInCell >= PRECISION_OFFSET) {

				if (Grid3d::GetInstance()->GetWaterCubeIfPresent(i) != nullptr)
					Grid3d::GetInstance()->SetWaterCubeVisibility(i, true);
				else
				{
					AWaterCube* newCube = GetWorld()->SpawnActor<AWaterCube>((FVector)*Grid3d::GetInstance()->GetCellPosition(i), *defaultRotation);
					Grid3d::GetInstance()->SetWaterCubeInTheGrid(newCube, i);
				}
				Grid3d::GetInstance()->SetNextIterationWaterLevel(i, summedWaterInCell);
			}
		}
		Grid3d::GetInstance()->SetWaterSpilt(i, 0.0f);
	}

	ApplyNextIterWaterToCurrent();
}
void AWorldController::ApplyNextIterWaterToCurrent() {
	for (int i = 0; i < N_CELLS; i++) {
		Grid3d::GetInstance()->SetWaterLevel(i, Grid3d::GetInstance()->GetNextIterationWaterLevel(i));
	}
}

void AWorldController::CalculateWaterCubeCapacity() {
	std::vector<int> indicesOfUnknownCapacity;
	std::vector<int> indicesInZStack;
	indicesInZStack.reserve(ZNCELLS);
	for (int i = 0; i < N_CELLS; i++) {
		AWaterCube* waterCube = Grid3d::GetInstance()->GetWaterCubeIfVisible(i);
		if (waterCube == nullptr)
			continue;

		waterCube->nextIterationWaterCapacity = BASE_CAPACITY;

		indicesInZStack.emplace_back(i);

		bool checkingUpwards = true;
		bool blockAbove = false;
		int topIndex = Grid3d::GetTopNeighborIndex(i);
		while (true) {
			if (!Grid3d::GetInstance()->CheckIfCellWIthinBounds(topIndex)) {
				break;
			}

			if (Grid3d::GetInstance()->CheckIfBlockCell(topIndex)) {
				blockAbove = true;
				break;
			}

			if (Grid3d::GetInstance()->GetWaterCubeIfVisible(topIndex) == nullptr) {
				break;
			}

			indicesInZStack.emplace_back(topIndex);
			waterCube->nextIterationWaterCapacity += EXCEED_MODIFIER;
			float currentLevel = Grid3d::GetInstance()->GetCurrentWaterLevel(topIndex) + Grid3d::GetInstance()->GetWaterSpilt(topIndex);
			if (!Grid3d::GetInstance()->CheckIfFullCapacityReached(topIndex, currentLevel)) {
				break;
			}
			topIndex = Grid3d::GetTopNeighborIndex(topIndex);
		}

		if (blockAbove) {
			for (int& idx : indicesInZStack) {
				Grid3d::GetInstance()->SetCapacityDetermined(idx, true);
				indicesOfUnknownCapacity.emplace_back(idx);
			}
		}
		else {
			for (int& idx : indicesInZStack)
				Grid3d::GetInstance()->SetCapacityDetermined(idx, false);
		}
		indicesInZStack.clear();
	}

	std::vector<int> toRemove;
	toRemove.reserve(indicesOfUnknownCapacity.size());

	while (!indicesOfUnknownCapacity.empty()) {
		for (int& idx : indicesOfUnknownCapacity) {

			if (SetByNeighbourWaterCapacityIfPresent(idx)) {
				toRemove.emplace_back(idx);
			}
			//UE_LOG(LogTemp, Warning, TEXT("toRemove.size(): %d"), toRemove.size());
		}
		if (toRemove.empty())
			return;

		while (!toRemove.empty()) {
			int idx = toRemove.back();
			toRemove.pop_back();
			indicesOfUnknownCapacity.erase(std::remove(indicesOfUnknownCapacity.begin(), indicesOfUnknownCapacity.end(), idx), indicesOfUnknownCapacity.end());
		}
		//UE_LOG(LogTemp, Warning, TEXT("indicesOfUnknownCapacity.size(): %d"), indicesOfUnknownCapacity.size());
	}


}

bool AWorldController::SetByNeighbourWaterCapacityIfPresent(const int& index) {
	float highestCapacity = 0.0f;
	bool isWaterAround = false;
	int leftIndex = Grid3d::GetLeftNeighborIndex(index);
	GetHigherCapacity(highestCapacity, leftIndex, isWaterAround);
	int frontIndex = Grid3d::GetFrontNeighborIndex(index);
	GetHigherCapacity(highestCapacity, frontIndex, isWaterAround);
	int rightIndex = Grid3d::GetRightNeighborIndex(index);
	GetHigherCapacity(highestCapacity, rightIndex, isWaterAround);
	int behindIndex = Grid3d::GetBehindNeighborIndex(index);
	GetHigherCapacity(highestCapacity, behindIndex, isWaterAround);
	if (highestCapacity > BASE_CAPACITY - PRECISION_OFFSET) {
		Grid3d::GetInstance()->SetNextIterationCapacity(index, highestCapacity);
		Grid3d::GetInstance()->SetCapacityDetermined(index, false);
		return true;
	}
	if (!isWaterAround) {
		Grid3d::GetInstance()->SetNextIterationCapacity(index, BASE_CAPACITY);
		Grid3d::GetInstance()->SetCapacityDetermined(index, false);
		return true;
	}
	return false;
}

void AWorldController::GetHigherCapacity(float& currCapacity, const int& index, bool& isWaterAround) {
	if (!Grid3d::GetInstance()->CheckIfCellWIthinBounds(index)) {
		return;
	}

	AWaterCube* waterCube = Grid3d::GetInstance()->GetWaterCubeIfVisible(index);
	if (waterCube == nullptr)
		return;
	isWaterAround = true;

	if (waterCube->isCapacityUndetermined)
		return;

	if (waterCube->nextIterationWaterCapacity > currCapacity)
		currCapacity = waterCube->nextIterationWaterCapacity;
}

void AWorldController::ApplyCalculatedCapacities() {
	for (int i = 0; i < N_CELLS; i++) {
		AWaterCube* waterCube = Grid3d::GetInstance()->GetWaterCubeIfVisible(i);
		if (waterCube != nullptr) {
			//UE_LOG(LogTemp, Warning, TEXT("waterCube->nextIterationWaterCapacity[%d]: %f"), i, waterCube->nextIterationWaterCapacity);
			waterCube->currentWaterCapacity = waterCube->nextIterationWaterCapacity;
			waterCube->nextIterationWaterCapacity = BASE_CAPACITY;
		}
	}
}
void AWorldController::ClusterizeWaterGroupsOnLevels() {
	//go through the cells from start to beginning
	//if water && not assigned to any cluster
	//do method TraverseAdjacentWaters(), that has to go like this:
	//1. set its cluster to some in the grid data

	//2. check four neighbours
	//3. if water and not assigned to cluster add them to waterCubesToTraverse and assign to the current cluster
	//4. otherwise do nothing

	//5. while (!needTraverseList.empty) {
	//  pop from needTraverselist
	// do step 3 }
	for (int ZLvl = 0; ZLvl < ZNCELLS; ZLvl++) {

		int currClusterNum = 0;
		for (int idx = ZLvl * XYNCELLS; idx < XYNCELLS * (ZLvl + 1); idx++)
		{
			AWaterCube* waterCube = Grid3d::GetInstance()->GetWaterCubeIfVisible(idx);
			if (waterCube == nullptr || waterCube->clusterNum > 0)
				continue;
			currClusterNum++;
			waterCube->clusterNum = currClusterNum;
			TraverseAdjacentWaters(currClusterNum, idx);

		}

		SetAllCapacitiesInClusterToHighest(currClusterNum, ZLvl * XYNCELLS, XYNCELLS * (ZLvl + 1));
	}

	ResetClusters();

}
void AWorldController::ResetClusters() {
	for (int idx = 0; idx < N_CELLS; idx++)
	{
		AWaterCube* waterCube = Grid3d::GetInstance()->GetWaterCubeIfVisible(idx);
		if (waterCube == nullptr)
			continue;
		waterCube->clusterNum = 0;
	}
}
void AWorldController::TraverseAdjacentWaters(const int& currentCluster, const int& startIdx) {

	std::vector<int> waterCubesToTraverse;
	waterCubesToTraverse.reserve(XYNCELLS);
	waterCubesToTraverse.emplace_back(startIdx);

	while (!waterCubesToTraverse.empty()) {
		int currIdx = waterCubesToTraverse.back();
		waterCubesToTraverse.pop_back();

		//check all 4 neighbours
		int leftIndex = Grid3d::GetLeftNeighborIndex(currIdx);
		if (Grid3d::GetInstance()->CheckIfCellWIthinBounds(leftIndex)) {
			AWaterCube* waterCube = Grid3d::GetInstance()->GetWaterCubeIfVisible(leftIndex);
			if (waterCube != nullptr && waterCube->clusterNum == 0) {
				waterCubesToTraverse.emplace_back(leftIndex);
				waterCube->clusterNum = currentCluster;
			}
		}

		int rightIndex = Grid3d::GetRightNeighborIndex(currIdx);
		if (Grid3d::GetInstance()->CheckIfCellWIthinBounds(rightIndex)) {
			AWaterCube* waterCube = Grid3d::GetInstance()->GetWaterCubeIfVisible(rightIndex);
			if (waterCube != nullptr && waterCube->clusterNum == 0) {
				waterCubesToTraverse.emplace_back(rightIndex);
				waterCube->clusterNum = currentCluster;
			}
		}
		int frontIndex = Grid3d::GetFrontNeighborIndex(currIdx);
		if (Grid3d::GetInstance()->CheckIfCellWIthinBounds(frontIndex)) {
			AWaterCube* waterCube = Grid3d::GetInstance()->GetWaterCubeIfVisible(frontIndex);
			if (waterCube != nullptr && waterCube->clusterNum == 0) {
				waterCubesToTraverse.emplace_back(frontIndex);
				waterCube->clusterNum = currentCluster;
			}
		}

		int behindIndex = Grid3d::GetBehindNeighborIndex(currIdx);
		if (Grid3d::GetInstance()->CheckIfCellWIthinBounds(behindIndex)) {
			AWaterCube* waterCube = Grid3d::GetInstance()->GetWaterCubeIfVisible(behindIndex);
			if (waterCube != nullptr && waterCube->clusterNum == 0) {
				waterCubesToTraverse.emplace_back(behindIndex);
				waterCube->clusterNum = currentCluster;
			}
		}

	}

}
void AWorldController::SetAllCapacitiesInClusterToHighest(const int& nClusters, const int& firstIdx, const int& lastIdx) {
	std::vector<float> capacitiesForClusters;
	capacitiesForClusters.reserve(nClusters + 1);
	for (int i = 0; i < nClusters; i++) {
		capacitiesForClusters.emplace_back(BASE_CAPACITY);
	}

	for (int i = firstIdx; i < lastIdx; i++) {
		AWaterCube* waterCube = Grid3d::GetInstance()->GetWaterCubeIfVisible(i);
		if (waterCube == nullptr)
			continue;
		if (capacitiesForClusters[waterCube->clusterNum - 1] < waterCube->currentWaterCapacity)
			capacitiesForClusters[waterCube->clusterNum - 1] = waterCube->currentWaterCapacity;
	}
	for (int i = firstIdx; i < lastIdx; i++) {
		AWaterCube* waterCube = Grid3d::GetInstance()->GetWaterCubeIfVisible(i);
		if (waterCube == nullptr)
			continue;

		waterCube->currentWaterCapacity = capacitiesForClusters[waterCube->clusterNum - 1];
	}
	capacitiesForClusters.clear();
}

void AWorldController::DetermineWaterFlow() {
	for (int i = 0; i < N_CELLS; i++) {
		AWaterCube* waterCube = Grid3d::GetInstance()->GetWaterCubeIfVisible(i);
		if (waterCube != nullptr) {
			float currentLevel = Grid3d::GetInstance()->GetCurrentWaterLevel(i) + Grid3d::GetInstance()->GetWaterSpilt(i);
			if (Grid3d::GetInstance()->CheckIfFullCapacityReached(i, currentLevel) && !CanWaterFallDown(i)) {
				EvaluateFlowFromNeighbours(i);
				//FlowAccordingToPressure(i);
			}

		}
	}
}

void AWorldController::FlowPressurizedWaterUpwards() {
/*
   * 1. if water in current cell and exceeded base capacity check the capacity of cell above
   * 2. if cell above is block do nothing
   * 3. if it is empty treat it as if it was of BASE_CAPACITY
   * 4. if water there get its capacity
   * 5. if the water cube above level not reached (current cell's capacity - Exceed Modifier) flow there
   * 6. amount to flow is std::clamp(Current Water Level - Base Capacity, 0.0f, Current Water Level - Base Capacity)
   *
   * */
	for (int i = 0; i < N_CELLS; i++) {
		AWaterCube* currWaterCube = Grid3d::GetInstance()->GetWaterCubeIfVisible(i);
		if (currWaterCube == nullptr)
			continue;
		if (Grid3d::GetInstance()->GetCurrentWaterLevel(i) + Grid3d::GetInstance()->GetWaterSpilt(i) < BASE_CAPACITY + PRECISION_OFFSET)
			continue;

		int topIndex = Grid3d::GetTopNeighborIndex(i);
		if (!Grid3d::GetInstance()->CheckIfCellWIthinBounds(topIndex) || Grid3d::GetInstance()->CheckIfBlockCell(topIndex))
			continue;
		//check if cube is pressurized
		float upWaterLevel = Grid3d::GetInstance()->GetCurrentWaterLevel(topIndex) + Grid3d::GetInstance()->GetWaterSpilt(topIndex);
		float upShouldBeCapacity = std::clamp(currWaterCube->currentWaterCapacity - EXCEED_MODIFIER, BASE_CAPACITY, currWaterCube->currentWaterCapacity - EXCEED_MODIFIER);
		float upFreeAmount = std::clamp(upShouldBeCapacity - upWaterLevel, 0.0f, upShouldBeCapacity - upWaterLevel);
		float waterToFlowUp = std::clamp(Grid3d::GetInstance()->GetCurrentWaterLevel(i) + Grid3d::GetInstance()->GetWaterSpilt(i) - (float)BASE_CAPACITY, 0.0f, upFreeAmount);
		//UE_LOG(LogTemp, Warning, TEXT("WaterToflowup(%d): %f"), topIndex, waterToFlowUp);

		Grid3d::GetInstance()->AddWaterSpilt(topIndex, waterToFlowUp);
		Grid3d::GetInstance()->AddWaterSpilt(i, -waterToFlowUp);
	}

}


void AWorldController::FlowAccordingToPressure(const int& index) {

	int leftIndex = Grid3d::GetLeftNeighborIndex(index);
	float leftOverload = GetWaterOverloadInCell(leftIndex);
	float leftWaterDiff = GetWaterAmountDiff(index, leftIndex);

	int rightIndex = Grid3d::GetRightNeighborIndex(index);
	float rightOverload = GetWaterOverloadInCell(rightIndex);
	float rightWaterDiff = GetWaterAmountDiff(index, rightIndex);

	//double XOverload = rightOverload - leftOverload;

	int frontIndex = Grid3d::GetFrontNeighborIndex(index);
	float frontOverload = GetWaterOverloadInCell(frontIndex);
	float frontWaterDiff = GetWaterAmountDiff(index, frontIndex);

	int behindIndex = Grid3d::GetBehindNeighborIndex(index);
	float behindOverload = GetWaterOverloadInCell(behindIndex);
	float behindWaterDiff = GetWaterAmountDiff(index, behindIndex);

	std::vector<int> neighboursToEven;
	neighboursToEven.reserve(4);

	if (leftWaterDiff > PRECISION_OFFSET)
		neighboursToEven.emplace_back(leftIndex);
	if (rightWaterDiff > PRECISION_OFFSET)
		neighboursToEven.emplace_back(rightIndex);
	if (frontWaterDiff > PRECISION_OFFSET)
		neighboursToEven.emplace_back(frontIndex);
	if (behindWaterDiff > PRECISION_OFFSET)
		neighboursToEven.emplace_back(behindIndex);

	float summedWater = Grid3d::GetInstance()->GetCurrentWaterLevel(index) + Grid3d::GetInstance()->GetWaterSpilt(index);
	for(int& idx : neighboursToEven)
	{
		summedWater += Grid3d::GetInstance()->GetCurrentWaterLevel(idx) + Grid3d::GetInstance()->GetWaterSpilt(idx);
	}

	float waterLvlForEach = summedWater / ((float)(neighboursToEven.size() + 1));
	std::vector<int> amountForIdx;
	amountForIdx.reserve(neighboursToEven.size());
	for (int& idx : neighboursToEven)
	{
		float amountToSpill = waterLvlForEach - (Grid3d::GetInstance()->GetCurrentWaterLevel(index) + Grid3d::GetInstance()->GetWaterSpilt(index));
		amountForIdx.emplace_back(amountToSpill);
	}
	float amountForCurrIdx = waterLvlForEach - (Grid3d::GetInstance()->GetCurrentWaterLevel(index) + Grid3d::GetInstance()->GetWaterSpilt(index));

	for (int i = 0; i < amountForIdx.size(); i++) {
		Grid3d::GetInstance()->AddWaterSpilt(neighboursToEven[i], amountForIdx[i]);
	}
	Grid3d::GetInstance()->AddWaterSpilt(index, amountForCurrIdx);

	neighboursToEven.clear();
	amountForIdx.clear();
	//double YOverload = frontOverload - behindWaterDiff;

	int topIndex = Grid3d::GetTopNeighborIndex(index);
	float topOverload = GetWaterOverloadInCell(topIndex);
	int bottomIndex = Grid3d::GetBottomNeighborIndex(index);
	float bottomOverload = GetWaterOverloadInCell(bottomIndex);

	double ZOverload = topOverload - bottomOverload;
	//CategoryName
	UE_LOG(LogTemp, Warning, TEXT("RLOverload[%d]: %f, %f, FB: %f, %f TB: %f, %f"), index, rightOverload, leftOverload, frontOverload, behindOverload, topOverload, bottomOverload);
	UE_LOG(LogTemp, Warning, TEXT("GetCurrentWaterLevel(%d): %f, GetWaterSpilt() %f, GetWaterCapacity(): %f"), index, Grid3d::GetInstance()->GetCurrentWaterLevel(index), Grid3d::GetInstance()->GetWaterSpilt(index), Grid3d::GetInstance()->GetWaterCapacity(index));

	double overloadedAmount = Grid3d::GetInstance()->GetCurrentWaterLevel(index) + Grid3d::GetInstance()->GetWaterSpilt(index) - Grid3d::GetInstance()->GetWaterCapacity(index);
	if (overloadedAmount < PRECISION_OFFSET)
		return;

	UE_LOG(LogTemp, Warning, TEXT("overloadedAmount[%d]: %f"), index, overloadedAmount);
	double amountToSpread = std::clamp(overloadedAmount, 0.0, MAX_PRESSURED_AMOUNT_ALLOWED_TO_SPREAD);

	double sumOfOverloadedNeighbours = std::fabs(ZOverload);

	if (sumOfOverloadedNeighbours < PRECISION_OFFSET)
		return;

	double amountToSpreadToZ = (ZOverload / sumOfOverloadedNeighbours) * amountToSpread;

	if (ZOverload < 0.0f) {
		Grid3d::GetInstance()->AddWaterSpilt(topIndex, amountToSpreadToZ);
	}
	else if (ZOverload > 0.0f) {
		Grid3d::GetInstance()->AddWaterSpilt(bottomIndex, amountToSpreadToZ);
	}
	Grid3d::GetInstance()->AddWaterSpilt(index, -amountToSpread);
}

void AWorldController::EvaluateFlowFromNeighbours(const int& index) {

	int leftIndex = Grid3d::GetLeftNeighborIndex(index);
	float leftOverload = GetWaterOverloadInCell(leftIndex);
	float leftWaterDiff = GetWaterAmountDiff(index, leftIndex);
	int rightIndex = Grid3d::GetRightNeighborIndex(index);
	float rightOverload = GetWaterOverloadInCell(rightIndex);
	float rightWaterDiff = GetWaterAmountDiff(index, rightIndex);

	double XOverload = rightOverload - leftOverload;

	int frontIndex = Grid3d::GetFrontNeighborIndex(index);
	float frontOverload = GetWaterOverloadInCell(frontIndex);
	float frontWaterDiff = GetWaterAmountDiff(index, frontIndex);

	int behindIndex = Grid3d::GetBehindNeighborIndex(index);
	float behindOverload = GetWaterOverloadInCell(behindIndex);
	float behindWaterDiff = GetWaterAmountDiff(index, behindIndex);

	double YOverload = frontOverload - behindWaterDiff;

	int topIndex = Grid3d::GetTopNeighborIndex(index);
	float topOverload = GetWaterOverloadInCell(topIndex);
	int bottomIndex = Grid3d::GetBottomNeighborIndex(index);
	float bottomOverload = GetWaterOverloadInCell(bottomIndex);

	double ZOverload = topOverload - bottomOverload;
	//CategoryName
	UE_LOG(LogTemp, Warning, TEXT("RLOverload[%d]: %f, %f, FB: %f, %f TB: %f, %f"), index, rightOverload, leftOverload, frontOverload, behindOverload, topOverload, bottomOverload);
	UE_LOG(LogTemp, Warning, TEXT("GetCurrentWaterLevel(%d): %f, GetWaterSpilt() %f, GetWaterCapacity(): %f"), index, Grid3d::GetInstance()->GetCurrentWaterLevel(index), Grid3d::GetInstance()->GetWaterSpilt(index), Grid3d::GetInstance()->GetWaterCapacity(index));

	double overloadedAmount = Grid3d::GetInstance()->GetCurrentWaterLevel(index) + Grid3d::GetInstance()->GetWaterSpilt(index) - Grid3d::GetInstance()->GetWaterCapacity(index);
	if (overloadedAmount < PRECISION_OFFSET)
		return;

	UE_LOG(LogTemp, Warning, TEXT("overloadedAmount[%d]: %f"), index, overloadedAmount);
	double amountToSpread = std::clamp(overloadedAmount, 0.0, MAX_PRESSURED_AMOUNT_ALLOWED_TO_SPREAD);

	double sumOfOverloadedNeighbours = std::fabs(XOverload) + std::fabs(YOverload);// + std::fabs(ZOverload);

	if (sumOfOverloadedNeighbours < PRECISION_OFFSET)
		return;

	double amountToSpreadToX = (XOverload / sumOfOverloadedNeighbours) * amountToSpread;
	double amountToSpreadToY = (YOverload / sumOfOverloadedNeighbours) * amountToSpread;
	double amountToSpreadToZ = (ZOverload / sumOfOverloadedNeighbours) * amountToSpread;
	//UE_LOG(LogTemp, Warning, TEXT("amountToSpreadToX: %f, amountToSpreadToY %f, amountToSpreadToZ %f"), amountToSpreadToX, amountToSpreadToY, amountToSpreadToZ);

	if (amountToSpreadToX < 0.0f) {
		Grid3d::GetInstance()->AddWaterSpilt(rightIndex, amountToSpreadToX);
	}
	else if (amountToSpreadToX > 0.0f) {
		Grid3d::GetInstance()->AddWaterSpilt(leftIndex, amountToSpreadToX);
	}

	if (amountToSpreadToY < 0.0f) {
		Grid3d::GetInstance()->AddWaterSpilt(frontIndex, amountToSpreadToY);
	}
	else if (amountToSpreadToY > 0.0f) {
		Grid3d::GetInstance()->AddWaterSpilt(behindIndex, amountToSpreadToY);
	}
	
	if (ZOverload < 0.0f) {
		Grid3d::GetInstance()->AddWaterSpilt(topIndex, amountToSpreadToZ);
	}
	else if (ZOverload > 0.0f) {
		Grid3d::GetInstance()->AddWaterSpilt(bottomIndex, amountToSpreadToZ);
	} 
	Grid3d::GetInstance()->AddWaterSpilt(index, -amountToSpread);

}

float AWorldController::GetWaterAmountDiff(const int& index, const int& neighbourIndex) {
	/*
	 if the return value is positive then water should flow to the neighbour
	*/

	if (Grid3d::GetInstance()->CheckIfCellWIthinBounds(neighbourIndex)) {
		AWaterCube* waterCube = Grid3d::GetInstance()->GetWaterCubeIfVisible(neighbourIndex);
		if (waterCube == nullptr && !Grid3d::GetInstance()->CheckIfBlockCell(neighbourIndex)) {
			return 0.0f;
		}
		if (waterCube != nullptr) {
			float neighWaterLevel = Grid3d::GetInstance()->GetCurrentWaterLevel(neighbourIndex) + Grid3d::GetInstance()->GetWaterSpilt(neighbourIndex);
			float currWaterLevel = Grid3d::GetInstance()->GetCurrentWaterLevel(index) + Grid3d::GetInstance()->GetWaterSpilt(index);
			return currWaterLevel - neighWaterLevel;
		}
	}
	return 0.0f;
}

float AWorldController::GetWaterOverloadInCell(const int& index) {
	if (Grid3d::GetInstance()->CheckIfCellWIthinBounds(index)) {
		AWaterCube* waterCube = Grid3d::GetInstance()->GetWaterCubeIfVisible(index);
		if (waterCube == nullptr && !Grid3d::GetInstance()->CheckIfBlockCell(index)) {
			return 0.1f;
		}
		if (waterCube != nullptr) {
			float overload = Grid3d::GetInstance()->CalculateWaterOverload(index);
			return overload;
		}
	}
	return 1.0f;
}
