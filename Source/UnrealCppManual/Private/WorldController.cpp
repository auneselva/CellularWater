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
	PrimaryActorTick.bCanEverTick = true;
	defaultRotation = new FRotator3d(0,0,0);

}

AWorldController::~AWorldController()
{
	delete defaultRotation;
}

void AWorldController::BeginPlay()
{
	wsgi = Cast<UWaterSimGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

	simThreshold = 1.0f / (float)wsgi->SimulationSpeed * 2.0f;
	moreOftenSimThreshold = simThreshold / 5.0f;
	Grid3d::GetInstance(*wsgi);
	Super::BeginPlay();

	gameTimeElapsed = 0.0f;
	moreOftenGameTimeElapsed = 0.0f;
	simCounter = 0;
}

void AWorldController::Tick(float DeltaTime) //delta time == around 0.02
{
	Super::Tick(DeltaTime);
	gameTimeElapsed += DeltaTime;
	moreOftenGameTimeElapsed += DeltaTime;

	if (gameTimeElapsed > simThreshold) {
		gameTimeElapsed -= simThreshold;
		ApplySimulationProccesses();
		simCounter++;
	}
}

void AWorldController::ApplySimulationProccesses() {

	for (int i = 0; i < Grid3d::GetInstance(*wsgi)->NCells; i++) {
		if (Grid3d::GetInstance(*wsgi)->GetWaterCubeIfVisible(i) != nullptr) {
			Gravity(i);
			SpillAround(i);
		}
		if (i % Grid3d::GetInstance(*wsgi)->xyNCells == Grid3d::GetInstance(*wsgi)->xyNCells - 1)
			HandleSpiltWater();
	}
	if (simCounter % 4 == 0) {

		CalculateWaterCubeCapacity();
		ApplyCalculatedCapacities();
		ClusterizeWaterGroupsOnLevels();
	}
	DetermineWaterFlow();
	HandleSpiltWater();
	FlowPressurizedWaterUpwards();
	//UE_LOG(LogTemp, Warning, TEXT("After Pressurizing"));

	HandleSpiltWater();
	Grid3d::GetInstance(*wsgi)->UpdateCubesTransform();

}

void AWorldController::Gravity(const int& index) {

	if (CanWaterFallDown(index)) {
		int bottomIndex = Grid3d::GetInstance(*wsgi)->GetBottomNeighborIndex(index);
		if (Grid3d::GetInstance(*wsgi)->GetWaterCubeIfVisible(bottomIndex) != nullptr) {

			float freeSpace = Grid3d::GetInstance(*wsgi)->GetWaterCapacity(bottomIndex) - Grid3d::GetInstance(*wsgi)->GetCurrentWaterLevel(bottomIndex) - Grid3d::GetInstance(*wsgi)->GetWaterSpilt(bottomIndex);
			freeSpace = std::clamp(freeSpace, 0.0f, freeSpace);
			float waterAmount = Grid3d::GetInstance(*wsgi)->GetCurrentWaterLevel(index) + Grid3d::GetInstance(*wsgi)->GetWaterSpilt(index);
			//if (waterAmount < 0.3 * BASE_CAPACITY && !Grid3d::GetInstance(*wsgi)->GetCanWaterBeFlowDownFromHere(index))
			//	return;
			
			float waterAmountToBeFlown = freeSpace;
			Grid3d::GetInstance(*wsgi)->AddWaterSpilt(bottomIndex, waterAmountToBeFlown);
			Grid3d::GetInstance(*wsgi)->AddWaterSpilt(index, -waterAmountToBeFlown);
		}
		else {
			Grid3d::GetInstance(*wsgi)->AddWaterSpilt(bottomIndex, Grid3d::GetInstance(*wsgi)->GetCurrentWaterLevel(index) + Grid3d::GetInstance(*wsgi)->GetWaterSpilt(index));
			Grid3d::GetInstance(*wsgi)->AddWaterSpilt(index, -Grid3d::GetInstance(*wsgi)->GetCurrentWaterLevel(index) - Grid3d::GetInstance(*wsgi)->GetWaterSpilt(index));
		}
	}
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
		int rightIndex = Grid3d::GetInstance(*wsgi)->GetRightNeighborIndex(index);

		if (IsNeighbourFreeToBeSpilledTo(index, rightIndex)) {
			sideNeighbours.emplace_back(rightIndex);
		}
		int leftIndex = Grid3d::GetInstance(*wsgi)->GetLeftNeighborIndex(index);
		if (IsNeighbourFreeToBeSpilledTo(index, leftIndex)) {
			sideNeighbours.emplace_back(leftIndex);
		}
		int frontIndex = Grid3d::GetInstance(*wsgi)->GetFrontNeighborIndex(index);
		if (IsNeighbourFreeToBeSpilledTo(index, frontIndex)) {
			sideNeighbours.emplace_back(frontIndex);
		}
		int behindIndex = Grid3d::GetInstance(*wsgi)->GetBehindNeighborIndex(index);
		if (IsNeighbourFreeToBeSpilledTo(index, behindIndex)) {
			sideNeighbours.emplace_back(behindIndex);
		}

		if (std::find(sideNeighbours.begin(), sideNeighbours.end(), frontIndex) != sideNeighbours.end()) {
			if (std::find(sideNeighbours.begin(), sideNeighbours.end(), rightIndex) != sideNeighbours.end()) {
				int frontRightIndex = Grid3d::GetInstance(*wsgi)->GetRightNeighborIndex(frontIndex);
				if (IsNeighbourFreeToBeSpilledTo(index, frontRightIndex))
					diagonalNeighbours.emplace_back(frontRightIndex);
			}	
		}

		if (std::find(sideNeighbours.begin(), sideNeighbours.end(), rightIndex) != sideNeighbours.end()) {
			if (std::find(sideNeighbours.begin(), sideNeighbours.end(), behindIndex) != sideNeighbours.end()) {
				int rightBehindIndex = Grid3d::GetInstance(*wsgi)->GetBehindNeighborIndex(rightIndex);
				if (IsNeighbourFreeToBeSpilledTo(index, rightBehindIndex))
					diagonalNeighbours.emplace_back(rightBehindIndex);
			}
		}

		if (std::find(sideNeighbours.begin(), sideNeighbours.end(), behindIndex) != sideNeighbours.end()) {
			if (std::find(sideNeighbours.begin(), sideNeighbours.end(), leftIndex) != sideNeighbours.end()) {
				int behindLeftIndex = Grid3d::GetInstance(*wsgi)->GetLeftNeighborIndex(behindIndex);
				if (IsNeighbourFreeToBeSpilledTo(index, behindLeftIndex))
					diagonalNeighbours.emplace_back(behindLeftIndex);
			}
		}

		if (std::find(sideNeighbours.begin(), sideNeighbours.end(), rightIndex) != sideNeighbours.end()) {
			if (std::find(sideNeighbours.begin(), sideNeighbours.end(), frontIndex) != sideNeighbours.end()) {
				int frontRightIndex = Grid3d::GetInstance(*wsgi)->GetFrontNeighborIndex(rightIndex);
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
			sideWaterSum += Grid3d::GetInstance(*wsgi)->GetCurrentWaterLevel(i) + Grid3d::GetInstance(*wsgi)->GetWaterSpilt(i);
		for (int& i : diagonalNeighbours)
			diagWaterSum += Grid3d::GetInstance(*wsgi)->GetCurrentWaterLevel(i) + Grid3d::GetInstance(*wsgi)->GetWaterSpilt(i);
		float sumWater = sideWaterSum + diagWaterSum + Grid3d::GetInstance(*wsgi)->GetCurrentWaterLevel(index) + Grid3d::GetInstance(*wsgi)->GetWaterSpilt(index);
		const int cells = sideNeighbours.size() + diagonalNeighbours.size() + 1;
		float waterAmountForEach = sumWater / cells;
		//UE_LOG(LogTemp, Warning, TEXT("waterAmountForEachNeighbour: %f"), waterAmountForEachNeighbour);
		for (int& i : sideNeighbours)
			Grid3d::GetInstance(*wsgi)->AddWaterSpilt(i, waterAmountForEach - (Grid3d::GetInstance(*wsgi)->GetCurrentWaterLevel(i) + Grid3d::GetInstance(*wsgi)->GetWaterSpilt(i)));
		for (int& i : diagonalNeighbours)
			Grid3d::GetInstance(*wsgi)->AddWaterSpilt(i, waterAmountForEach - (Grid3d::GetInstance(*wsgi)->GetCurrentWaterLevel(i) + Grid3d::GetInstance(*wsgi)->GetWaterSpilt(i)));
		Grid3d::GetInstance(*wsgi)->AddWaterSpilt(index, waterAmountForEach - (Grid3d::GetInstance(*wsgi)->GetCurrentWaterLevel(index) + Grid3d::GetInstance(*wsgi)->GetWaterSpilt(index)));
	}
}
bool AWorldController::IsNeighbourFreeToBeSpilledTo(const int& currentIndex, const int& neighbourIndex) {
	if (Grid3d::GetInstance(*wsgi)->CheckIfCellWIthinBounds(neighbourIndex)) {
		if (Grid3d::GetInstance(*wsgi)->CheckIfBlockCell(neighbourIndex))
			return false;
		if (Grid3d::GetInstance(*wsgi)->GetWaterCubeIfVisible(neighbourIndex) == nullptr)
			return true;
		if (Grid3d::GetInstance(*wsgi)->GetCurrentWaterLevel(neighbourIndex) + Grid3d::GetInstance(*wsgi)->GetWaterSpilt(neighbourIndex) < Grid3d::GetInstance(*wsgi)->GetCurrentWaterLevel(currentIndex) + Grid3d::GetInstance(*wsgi)->GetWaterSpilt(currentIndex))
			return true;
	}
	return false;
}
bool AWorldController::CanWaterFallDown(const int& currentIndex) {
	int bottomIndex = Grid3d::GetInstance(*wsgi)->GetBottomNeighborIndex(currentIndex);
	if (Grid3d::GetInstance(*wsgi)->CheckIfCellWIthinBounds(bottomIndex)) {
		if (Grid3d::GetInstance(*wsgi)->CheckIfBlockCell(bottomIndex))
			return false;
		if (Grid3d::GetInstance(*wsgi)->GetWaterCubeIfVisible(bottomIndex) == nullptr)
			return true;
		if (!Grid3d::GetInstance(*wsgi)->CheckIfFullCapacityReached(bottomIndex, Grid3d::GetInstance(*wsgi)->GetWaterSpilt(bottomIndex) + Grid3d::GetInstance(*wsgi)->GetCurrentWaterLevel(bottomIndex)))
			return true;
	}
	return false;
}

void AWorldController::HandleSpiltWater() { //or handle pressure?
	for (int i = 0; i < Grid3d::GetInstance(*wsgi)->NCells; i++) {
		if (!Grid3d::GetInstance(*wsgi)->CheckIfBlockCell(i)) {
			float summedWaterInCell = Grid3d::GetInstance(*wsgi)->GetWaterSpilt(i) + Grid3d::GetInstance(*wsgi)->GetCurrentWaterLevel(i);
			if (summedWaterInCell < PRECISION_OFFSET) {
				Grid3d::GetInstance(*wsgi)->SetWaterCubeVisibility(i, false);
				Grid3d::GetInstance(*wsgi)->SetNextIterationWaterLevel(i, 0.0f);
			}
			else if (summedWaterInCell >= PRECISION_OFFSET) {

				if (Grid3d::GetInstance(*wsgi)->GetWaterCubeIfPresent(i) != nullptr)
					Grid3d::GetInstance(*wsgi)->SetWaterCubeVisibility(i, true);
				else
				{
					AWaterCube* newCube = GetWorld()->SpawnActor<AWaterCube>((FVector)*Grid3d::GetInstance(*wsgi)->GetCellPosition(i), *defaultRotation);
					Grid3d::GetInstance(*wsgi)->SetWaterCubeInTheGrid(newCube, i);
				}
				Grid3d::GetInstance(*wsgi)->SetNextIterationWaterLevel(i, summedWaterInCell);
			}
		}
		Grid3d::GetInstance(*wsgi)->SetWaterSpilt(i, 0.0f);
	}

	ApplyNextIterWaterToCurrent();
}
void AWorldController::ApplyNextIterWaterToCurrent() {
	for (int i = 0; i < Grid3d::GetInstance(*wsgi)->NCells; i++) {
		Grid3d::GetInstance(*wsgi)->SetWaterLevel(i, Grid3d::GetInstance(*wsgi)->GetNextIterationWaterLevel(i));
	}
}

void AWorldController::CalculateWaterCubeCapacity() {
	std::vector<int> indicesOfUnknownCapacity;
	std::vector<int> indicesInZStack;
	indicesInZStack.reserve(Grid3d::GetInstance(*wsgi)->zNCells);
	for (int i = 0; i < Grid3d::GetInstance(*wsgi)->NCells; i++) {
		AWaterCube* waterCube = Grid3d::GetInstance(*wsgi)->GetWaterCubeIfVisible(i);
		if (waterCube == nullptr)
			continue;

		waterCube->nextIterationWaterCapacity = BASE_CAPACITY;

		indicesInZStack.emplace_back(i);

		bool checkingUpwards = true;
		bool blockAbove = false;
		int topIndex = Grid3d::GetInstance(*wsgi)->GetTopNeighborIndex(i);
		while (true) {
			if (!Grid3d::GetInstance(*wsgi)->CheckIfCellWIthinBounds(topIndex)) {
				break;
			}

			if (Grid3d::GetInstance(*wsgi)->CheckIfBlockCell(topIndex)) {
				blockAbove = true;
				break;
			}

			if (Grid3d::GetInstance(*wsgi)->GetWaterCubeIfVisible(topIndex) == nullptr) {
				break;
			}

			indicesInZStack.emplace_back(topIndex);
			waterCube->nextIterationWaterCapacity += EXCEED_MODIFIER;
			float currentLevel = Grid3d::GetInstance(*wsgi)->GetCurrentWaterLevel(topIndex) + Grid3d::GetInstance(*wsgi)->GetWaterSpilt(topIndex);
			if (!Grid3d::GetInstance(*wsgi)->CheckIfFullCapacityReached(topIndex, currentLevel)) {
				break;
			}
			topIndex = Grid3d::GetInstance(*wsgi)->GetTopNeighborIndex(topIndex);
		}

		if (blockAbove) {
			for (int& idx : indicesInZStack) {
				Grid3d::GetInstance(*wsgi)->SetCapacityDetermined(idx, true);
				indicesOfUnknownCapacity.emplace_back(idx);
			}
		}
		else {
			for (int& idx : indicesInZStack)
				Grid3d::GetInstance(*wsgi)->SetCapacityDetermined(idx, false);
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
	std::vector<int> neighbourIndices = Get4HorizontalNeighbourIndices(index);

	for (int& neighIdx : neighbourIndices) {
		GetHigherCapacity(highestCapacity, neighIdx, isWaterAround);
	}

	if (highestCapacity > BASE_CAPACITY - PRECISION_OFFSET) {
		Grid3d::GetInstance(*wsgi)->SetNextIterationCapacity(index, highestCapacity);
		Grid3d::GetInstance(*wsgi)->SetCapacityDetermined(index, false);
		return true;
	}
	if (!isWaterAround) {
		Grid3d::GetInstance(*wsgi)->SetNextIterationCapacity(index, BASE_CAPACITY);
		Grid3d::GetInstance(*wsgi)->SetCapacityDetermined(index, false);
		return true;
	}
	return false;
}

void AWorldController::GetHigherCapacity(float& currCapacity, const int& index, bool& isWaterAround) {
	if (!Grid3d::GetInstance(*wsgi)->CheckIfCellWIthinBounds(index)) {
		return;
	}

	AWaterCube* waterCube = Grid3d::GetInstance(*wsgi)->GetWaterCubeIfVisible(index);
	if (waterCube == nullptr)
		return;
	isWaterAround = true;

	if (waterCube->isCapacityUndetermined)
		return;

	if (waterCube->nextIterationWaterCapacity > currCapacity)
		currCapacity = waterCube->nextIterationWaterCapacity;
}

void AWorldController::ApplyCalculatedCapacities() {
	for (int i = 0; i < Grid3d::GetInstance(*wsgi)->NCells; i++) {
		AWaterCube* waterCube = Grid3d::GetInstance(*wsgi)->GetWaterCubeIfVisible(i);
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
	for (int ZLvl = 0; ZLvl < Grid3d::GetInstance(*wsgi)->zNCells; ZLvl++) {

		int currClusterNum = 0;
		for (int idx = ZLvl * Grid3d::GetInstance(*wsgi)->xyNCells; idx < Grid3d::GetInstance(*wsgi)->xyNCells * (ZLvl + 1); idx++)
		{
			AWaterCube* waterCube = Grid3d::GetInstance(*wsgi)->GetWaterCubeIfVisible(idx);
			if (waterCube == nullptr || waterCube->clusterNum > 0)
				continue;
			currClusterNum++;
			waterCube->clusterNum = currClusterNum;
			TraverseAdjacentWaters(currClusterNum, idx);

		}

		SetAllCapacitiesInClusterToHighest(currClusterNum, ZLvl * Grid3d::GetInstance(*wsgi)->xyNCells, Grid3d::GetInstance(*wsgi)->xyNCells * (ZLvl + 1));
	}

	ResetClusters();

}
void AWorldController::ResetClusters() {
	for (int idx = 0; idx < Grid3d::GetInstance(*wsgi)->NCells; idx++)
	{
		AWaterCube* waterCube = Grid3d::GetInstance(*wsgi)->GetWaterCubeIfVisible(idx);
		if (waterCube == nullptr)
			continue;
		waterCube->clusterNum = 0;
	}
}
void AWorldController::TraverseAdjacentWaters(const int& currentCluster, const int& startIdx) {

	std::vector<int> waterCubesToTraverse;
	waterCubesToTraverse.reserve(Grid3d::GetInstance(*wsgi)->xyNCells);
	waterCubesToTraverse.emplace_back(startIdx);

	while (!waterCubesToTraverse.empty()) {
		int currIdx = waterCubesToTraverse.back();
		waterCubesToTraverse.pop_back();

		//check all 4 neighbours' clusters

		std::vector<int> neigbourIndices = Get4HorizontalNeighbourIndices(currIdx);
		for (int& neighIdx : neigbourIndices) {

			if (Grid3d::GetInstance(*wsgi)->CheckIfCellWIthinBounds(neighIdx)) {
				AWaterCube* waterCube = Grid3d::GetInstance(*wsgi)->GetWaterCubeIfVisible(neighIdx);
				if (waterCube != nullptr && waterCube->clusterNum == 0) {
					waterCubesToTraverse.emplace_back(neighIdx);
					waterCube->clusterNum = currentCluster;
				}
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
		AWaterCube* waterCube = Grid3d::GetInstance(*wsgi)->GetWaterCubeIfVisible(i);
		if (waterCube == nullptr)
			continue;
		if (capacitiesForClusters[waterCube->clusterNum - 1] < waterCube->currentWaterCapacity)
			capacitiesForClusters[waterCube->clusterNum - 1] = waterCube->currentWaterCapacity;
	}
	for (int i = firstIdx; i < lastIdx; i++) {
		AWaterCube* waterCube = Grid3d::GetInstance(*wsgi)->GetWaterCubeIfVisible(i);
		if (waterCube == nullptr)
			continue;

		waterCube->currentWaterCapacity = capacitiesForClusters[waterCube->clusterNum - 1];
	}
	capacitiesForClusters.clear();
}

void AWorldController::DetermineWaterFlow() {
	for (int i = 0; i < Grid3d::GetInstance(*wsgi)->NCells; i++) {
		AWaterCube* waterCube = Grid3d::GetInstance(*wsgi)->GetWaterCubeIfVisible(i);
		if (waterCube != nullptr) {
			float currentLevel = Grid3d::GetInstance(*wsgi)->GetCurrentWaterLevel(i) + Grid3d::GetInstance(*wsgi)->GetWaterSpilt(i);
			if (Grid3d::GetInstance(*wsgi)->CheckIfFullCapacityReached(i, currentLevel) && !CanWaterFallDown(i)) {
				EvaluateFlowFromNeighbours(i);
				//FlowAccordingToPressure(i);
			}

		}
		//if (i % Grid3d::GetInstance(*wsgi)->xyNCells == Grid3d::GetInstance(*wsgi)->xyNCells - 1)
		//	HandleSpiltWater();
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
	for (int i = 0; i < Grid3d::GetInstance(*wsgi)->NCells; i++) {
		AWaterCube* currWaterCube = Grid3d::GetInstance(*wsgi)->GetWaterCubeIfVisible(i);
		if (currWaterCube == nullptr)
			continue;
		if (Grid3d::GetInstance(*wsgi)->GetCurrentWaterLevel(i) + Grid3d::GetInstance(*wsgi)->GetWaterSpilt(i) < BASE_CAPACITY + PRECISION_OFFSET) {
			continue;
		}

		int topIndex = Grid3d::GetInstance(*wsgi)->GetTopNeighborIndex(i);
		if (!Grid3d::GetInstance(*wsgi)->CheckIfCellWIthinBounds(topIndex) || Grid3d::GetInstance(*wsgi)->CheckIfBlockCell(topIndex))
			continue;

		//check if cube is pressurized
		float upWaterLevel = Grid3d::GetInstance(*wsgi)->GetCurrentWaterLevel(topIndex) + Grid3d::GetInstance(*wsgi)->GetWaterSpilt(topIndex);
		float upShouldBeCapacity = std::clamp(currWaterCube->currentWaterCapacity - EXCEED_MODIFIER, BASE_CAPACITY, currWaterCube->currentWaterCapacity - EXCEED_MODIFIER);
		float upFreeAmount = std::clamp(upShouldBeCapacity - upWaterLevel, 0.0f, upShouldBeCapacity - upWaterLevel);
		float waterToFlowUp = std::clamp(Grid3d::GetInstance(*wsgi)->GetCurrentWaterLevel(i) + Grid3d::GetInstance(*wsgi)->GetWaterSpilt(i) - (float)(BASE_CAPACITY + EXCEED_MODIFIER), 0.0f, upFreeAmount);
		//UE_LOG(LogTemp, Warning, TEXT("[%d]upwater: %f"), i, waterToFlowUp);

		Grid3d::GetInstance(*wsgi)->AddWaterSpilt(topIndex, waterToFlowUp);
		Grid3d::GetInstance(*wsgi)->AddWaterSpilt(i, -waterToFlowUp);

	}

}

void AWorldController::EvaluateFlowFromNeighbours(const int& index) {
	/*
	if cell is overloaded with water spread to others
	1. if neighbour is not a block cell flow there
	2. how much? calculate free space in all neighbours and spread the overflowing amount in equal and flow there

	*/

	double overloadedAmount = Grid3d::GetInstance(*wsgi)->GetCurrentWaterLevel(index) - Grid3d::GetInstance(*wsgi)->GetWaterCapacity(index);
	if (overloadedAmount < PRECISION_OFFSET)
		return;

	std::vector<int> neighsIdxToSpreadTo;
	std::vector<float> spaceInNeighsToSpreadTo;
	neighsIdxToSpreadTo.reserve(6);
	spaceInNeighsToSpreadTo.reserve(6);
	float summedSpaceInNeighs = 0;

	std::vector<int> neighbourIndices = Get6AdjacentNeighbourIndices(index);

	for (int& idx : neighbourIndices) {
		float freeAmount = GetFreeAmountInCell(idx);
		if (freeAmount > -PRECISION_OFFSET) { // free amount is set to -1.0f if the cell is out of world bounds or is a block
			neighsIdxToSpreadTo.emplace_back(idx);
			spaceInNeighsToSpreadTo.emplace_back(freeAmount);
			summedSpaceInNeighs += freeAmount;
		}

	}

	//UE_LOG(LogTemp, Warning, TEXT("overloadedAmount[%d]: %f"), index, overloadedAmount);
	double amountToSpread = std::clamp(overloadedAmount, 0.0, MAX_PRESSURED_AMOUNT_ALLOWED_TO_SPREAD);

	if (summedSpaceInNeighs < PRECISION_OFFSET * 10) {
		float amountToSpreadToEach = amountToSpread / (double)neighsIdxToSpreadTo.size();
		//UE_LOG(LogTemp, Warning, TEXT("amountToSpreadToX: %f, amountToSpreadToY %f, amountToSpreadToZ %f"), amountToSpreadToX, amountToSpreadToY, amountToSpreadToZ);
		float sumSpilt = 0.0f;
		for (int i = 0; i < neighsIdxToSpreadTo.size(); i++) {
			float toSpill = std::clamp(amountToSpreadToEach, 0.0f, amountToSpreadToEach);
			sumSpilt += toSpill;
			Grid3d::GetInstance(*wsgi)->AddWaterSpilt(neighsIdxToSpreadTo[i], toSpill);
		}
		Grid3d::GetInstance(*wsgi)->AddWaterSpilt(index, -sumSpilt);
	}
	else {
		float sumSpilt = 0.0f;
		for (int i = 0; i < neighsIdxToSpreadTo.size(); i++) {
			float toSpill = spaceInNeighsToSpreadTo[i] / summedSpaceInNeighs * amountToSpread;
			sumSpilt += toSpill;
			Grid3d::GetInstance(*wsgi)->AddWaterSpilt(neighsIdxToSpreadTo[i], toSpill);
		}
		Grid3d::GetInstance(*wsgi)->AddWaterSpilt(index, -sumSpilt);
	}


}

float AWorldController::GetFreeAmountInCell(const int& index) {
	if (!Grid3d::GetInstance(*wsgi)->CheckIfCellWIthinBounds(index))
		return -1.0f;
	if (Grid3d::GetInstance(*wsgi)->CheckIfBlockCell(index))
		return -1.0f;

	AWaterCube* waterCube = Grid3d::GetInstance(*wsgi)->GetWaterCubeIfVisible(index);
	if (waterCube == nullptr) {
		return 1.0f;
	}
	if (waterCube != nullptr) {
		double overload = Grid3d::GetInstance(*wsgi)->GetCurrentWaterLevel(index) + Grid3d::GetInstance(*wsgi)->GetWaterSpilt(index) - Grid3d::GetInstance(*wsgi)->GetWaterCapacity(index);
		return std::clamp(overload, 0.0, MAX_PRESSURED_AMOUNT_ALLOWED_TO_SPREAD);
	}
	return -1.0f;
}

std::vector<int> AWorldController::Get4HorizontalNeighbourIndices(const int& index) {
	std::vector<int> neighs;
	neighs.reserve(4);
	neighs.emplace_back(Grid3d::GetInstance(*wsgi)->GetLeftNeighborIndex(index));
	neighs.emplace_back(Grid3d::GetInstance(*wsgi)->GetFrontNeighborIndex(index));
	neighs.emplace_back(Grid3d::GetInstance(*wsgi)->GetRightNeighborIndex(index));
	neighs.emplace_back(Grid3d::GetInstance(*wsgi)->GetBehindNeighborIndex(index));
	return std::move(neighs);
}

std::vector<int> AWorldController::Get6AdjacentNeighbourIndices(const int& index) {
	std::vector<int> neighs;
	neighs.reserve(6);
	neighs.emplace_back(Grid3d::GetInstance(*wsgi)->GetLeftNeighborIndex(index));
	neighs.emplace_back(Grid3d::GetInstance(*wsgi)->GetFrontNeighborIndex(index));
	neighs.emplace_back(Grid3d::GetInstance(*wsgi)->GetRightNeighborIndex(index));
	neighs.emplace_back(Grid3d::GetInstance(*wsgi)->GetBehindNeighborIndex(index));
	neighs.emplace_back(Grid3d::GetInstance(*wsgi)->GetTopNeighborIndex(index));
	neighs.emplace_back(Grid3d::GetInstance(*wsgi)->GetBottomNeighborIndex(index));
	return std::move(neighs);
}
