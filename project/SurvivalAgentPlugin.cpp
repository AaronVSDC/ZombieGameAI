#include "stdafx.h"
#include "SurvivalAgentPlugin.h"
#include "IExamInterface.h"

using namespace std;

//Called only once, during initialization
void SurvivalAgentPlugin::Initialize(IBaseInterface* pInterface, PluginInfo& info)
{
	m_pInterface = static_cast<IExamInterface*>(pInterface);

	info.BotName = "mrHeylen";
	info.Student_Name = "Aaron_Van_Sichem_De_Combe"; 
	info.Student_Class = "2DAE19";
	info.LB_Password = "WalterWhite";

	//--------
	//INIT
	//--------
	m_pFSM = make_unique<FiniteStateMachine>(m_pInterface);
}

//Called only once
void SurvivalAgentPlugin::DllInit()
{
	//Called when the plugin is loaded
}

//Called only once
void SurvivalAgentPlugin::DllShutdown()
{
	//Called when the plugin gets unloaded
}

void SurvivalAgentPlugin::InitGameDebugParams(GameDebugParams& params)
{
	params.AutoFollowCam = true; //Automatically follow the AI? (Default = true)
	params.RenderUI = true; //Render the IMGUI Panel? (Default = true)
	params.SpawnEnemies = true; //Do you want to spawn enemies? (Default = true)
	params.EnemyCount = 20; //How many enemies? (Default = 20)
	//params.GodMode = false; //GodMode > You can't die, can be useful to inspect certain behaviors (Default = false)
	//params.LevelFile = "GameLevel.gppl";
	//params.AutoGrabClosestItem = true; //A call to Item_Grab(...) returns the closest item that can be grabbed. (EntityInfo argument is ignored)
	//params.StartingDifficultyStage = 1;
	//params.InfiniteStamina = false;
	params.SpawnDebugPistol = true;
	//params.SpawnDebugShotgun = true;
	params.SpawnPurgeZonesOnMiddleClick = true;
	params.PrintDebugMessages = true;
	params.ShowDebugItemNames = true;
	params.Seed = int(time(nullptr)); //-1 = don't set seed. Any other number = fixed seed //TIP: use Seed = int(time(nullptr)) for pure randomness
}

void SurvivalAgentPlugin::Update_Debug(float dt)
{


	////Demo Event Code
	////In the end your Agent should be able to walk around without external input
	//if (m_pInterface->Input_IsMouseButtonUp(Elite::InputMouseButton::eLeft))
	//{
	//	//Update_Debug target based on input
	//	Elite::MouseData mouseData = m_pInterface->Input_GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eLeft);
	//	const Elite::Vector2 pos = Elite::Vector2(static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y));
	//	m_Target = m_pInterface->Debug_ConvertScreenToWorld(pos);
	//}
	//else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Space))
	//{
	//	m_CanRun = true;
	//}
	//else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Left))
	//{
	//	m_AngSpeed -= Elite::ToRadians(10);
	//}
	//else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Right))
	//{
	//	m_AngSpeed += Elite::ToRadians(10);
	//}
	//else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_G))
	//{
	//	m_GrabItem = true;
	//}
	//else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_U))
	//{
	//	m_UseItem = true;
	//}
	//else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_R))
	//{
	//	m_RemoveItem = true;
	//}
	//else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_X))
	//{
	//	m_DestroyItemsInFOV = true;
	//}
	//else if (m_pInterface->Input_IsKeyboardKeyUp(Elite::eScancode_Space))
	//{
	//	m_CanRun = false;
	//}
	//else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Delete))
	//{
	//	m_pInterface->RequestShutdown();
	//}
	//else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_KP_Minus))
	//{
	//	if (m_InventorySlot > 0)
	//		--m_InventorySlot;
	//}
	//else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_KP_Plus))
	//{
	//	if (m_InventorySlot < 4)
	//		++m_InventorySlot;
	//}
	//else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Q))
	//{
	//	ItemInfo info = {};
	//	m_pInterface->Inventory_GetItem(m_InventorySlot, info);
	//	std::cout << (int)info.Type << std::endl;
	//}
}

//This function calculates the new SteeringOutput, called once per frame
SteeringPlugin_Output SurvivalAgentPlugin::UpdateSteering(float dt)
{
	auto steering = SteeringPlugin_Output();

	return m_pFSM->Update(dt);


	////Use the Interface (IAssignmentInterface) to 'interface' with the AI_Framework
	//auto agentInfo = m_pInterface->Agent_GetInfo();


	////Use the navmesh to calculate the next navmesh point
	////auto nextTargetPos = m_pInterface->NavMesh_GetClosestPathPoint(checkpointLocation);

	//m_Target = Elite::Vector2(5.f, 0.f); 
	////OR, Use the mouse target
	//auto nextTargetPos = m_pInterface->NavMesh_GetClosestPathPoint(m_Target);

	////FOV USAGE DEMO
	////===============

	////FOV stats = CHEAP! info about the FOV
	//FOVStats stats = m_pInterface->FOV_GetStats();

	//
	//

	////FOV data (snapshot of the FOV of the current frame) = EXPENSIVE! returns a new vector for every call
	//auto vHousesInFOV = m_pInterface->GetHousesInFOV();
	//auto vEnemiesInFOV = m_pInterface->GetEnemiesInFOV();
	//auto vItemsInFOV = m_pInterface->GetItemsInFOV();
	//auto vPurgezonesInFOV = m_pInterface->GetPurgeZonesInFOV();

	//for (auto& zoneInfo : vPurgezonesInFOV)
	//{
	//	std::cout << "Purge Zone in FOV:" << zoneInfo.Center.x << ", "<< zoneInfo.Center.y << "---Radius: "<< zoneInfo.Radius << std::endl;
	//}

	//for (auto& enemyInfo : vEnemiesInFOV)
	//{
	//	std::cout << "Enemy in FOV:" << enemyInfo.Location.x << ", " << enemyInfo.Location.y << "---Health: " << enemyInfo.Health << std::endl;
	//}

	//for (auto& item : vItemsInFOV)
	//{
	//	std::cout << "Item in FOV:" << item.Location.x << ", " << item.Location.y << "---Value: " << item.Value << std::endl;	
	//}

	//for (auto& house : vHousesInFOV)
	//{
	//	std::cout << "House in FOV:" << house.Center.x << ", " << house.Center.y << "---Size: " << house.Size << std::endl;
	//}

	////INVENTORY USAGE DEMO
	////********************

	//if (m_GrabItem)
	//{
	//	ItemInfo item;
	//	//Item_Grab > When DebugParams.AutoGrabClosestItem is TRUE, the Item_Grab function returns the closest item in range
	//	//Keep in mind that DebugParams are only used for debugging purposes, by default this flag is FALSE
	//	//Otherwise, use GetEntitiesInFOV() to retrieve a vector of all entities in the FOV (EntityInfo)
	//	//Item_Grab gives you the ItemInfo back, based on the passed EntityHash (retrieved by GetEntitiesInFOV)
	//	if (m_pInterface->GrabNearestItem(item))
	//	{
	//		//Once grabbed, you can add it to a specific inventory slot
	//		//Slot must be empty
	//		m_pInterface->Inventory_AddItem(m_InventorySlot, item);
	//	}
	//}

	//if (m_UseItem)
	//{
	//	//Use an item (make sure there is an item at the given inventory slot)
	//	m_pInterface->Inventory_UseItem(m_InventorySlot);
	//}

	//if (m_RemoveItem)
	//{
	//	//Remove an item from a inventory slot
	//	m_pInterface->Inventory_RemoveItem(m_InventorySlot);
	//}

	//if (m_DestroyItemsInFOV)
	//{
	//	for (auto& item : vItemsInFOV)
	//	{
	//		m_pInterface->DestroyItem(item);
	//	}
	//}

	////Simple Seek Behaviour (towards Target)


	//steering.LinearVelocity *= agentInfo.MaxLinearSpeed; //Rescale to Max Speed

	////if (Distance(nextTargetPos, agentInfo.Position) < 2.f)
	////{
	////	steering.LinearVelocity = Elite::ZeroVector2;
	////}

	////steering.AngularVelocity = m_AngSpeed; //Rotate your character to inspect the world while walking

	//steering.AutoOrient = true; //Setting AutoOrient to true overrides the AngularVelocity
	//steering.RunMode = m_CanRun; //If RunMode is True > MaxLinearSpeed is increased for a limited time (until your stamina runs out)

	////SteeringPlugin_Output is works the exact same way a SteeringBehaviour output

	////@End (Demo Purposes)

	//m_GrabItem = false; //Reset State
	//m_UseItem = false;
	//m_RemoveItem = false;
	//m_DestroyItemsInFOV = false;

}

//This function should only be used for rendering debug elements
void SurvivalAgentPlugin::Render(float dt) const
{
	//This Render function should only contain calls to Interface->Draw_... functions
	m_pInterface->Draw_SolidCircle(m_Target, .7f, { 0,0 }, { 1, 0, 0 });

	m_pFSM->DebugRender(); 
}


