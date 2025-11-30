#include "NetworkObject.h"
#include "./enet/enet.h"
using namespace NCL;
using namespace CSC8503;

NetworkObject::NetworkObject(GameObject &o, int id) : object(o) {
  deltaErrors = 0;
  fullErrors = 0;
  networkID = id;
}

NetworkObject::~NetworkObject() {}

bool NetworkObject::ReadPacket(GamePacket &p) {
  switch (p.type) {
  case static_cast<uint16_t>(BasicNetworkMessages::Delta_State): {
    return ReadDeltaPacket(GamePacket::as<DeltaPacket>(p));
  }
  case static_cast<uint16_t>(BasicNetworkMessages::Full_State): {
    return ReadFullPacket(GamePacket::as<FullPacket>(p));
  }
  default:
    return false;
  }
}

bool NetworkObject::WritePacket(GamePacket **p, bool deltaFrame, int stateID) {
  if (deltaFrame) {
    if (!WriteDeltaPacket(p, stateID)) {
      return WriteFullPacket(p);
    }
  }
  return WriteFullPacket(p);
}
// Client objects recieve these packets
bool NetworkObject::ReadDeltaPacket(DeltaPacket &p) {
  if (p.fullID != lastFullState.stateID)
    return false;

  UpdateStateHistory(p.fullID);

  auto fullPos = lastFullState.position;
  auto fullRot = lastFullState.orientation;

  fullPos.x += (float)(p.pos[0]);
  fullPos.y += (float)(p.pos[1]);
  fullPos.z += (float)(p.pos[2]);

  constexpr float inv127 = 1.f / 127.f;

  fullRot.x += (float)(p.orientation[0] * inv127);
  fullRot.y += (float)(p.orientation[1] * inv127);
  fullRot.z += (float)(p.orientation[2] * inv127);
  fullRot.w += (float)(p.orientation[3] * inv127);

  object.GetTransform().SetPosition(fullPos).SetOrientation(fullRot);

  return true;
}

bool NetworkObject::ReadFullPacket(FullPacket &p) {
  if (p.fullState.stateID < lastFullState.stateID)
    return false;

  lastFullState = p.fullState;

  object.GetTransform()
      .SetPosition(lastFullState.position)
      .SetOrientation(lastFullState.orientation);

  stateHistory.push_back(lastFullState);

  return true;
}

bool NetworkObject::WriteDeltaPacket(GamePacket **p, int stateID) {
  DeltaPacket d = DeltaPacket();

  NetworkState state;
  if (!GetNetworkState(stateID, state))
    return false;

  d.fullID = stateID;
  d.objectID = networkID;

  auto currPos = object.GetTransform().GetPosition() - state.position;
  auto currRot = object.GetTransform().GetOrientation() - state.orientation;

  d.pos[0] = (char)(currPos.x);
  d.pos[1] = (char)(currPos.y);
  d.pos[2] = (char)(currPos.z);

  d.orientation[0] = (char)(currRot.x * 127.f);
  d.orientation[1] = (char)(currRot.y * 127.f);
  d.orientation[2] = (char)(currRot.z * 127.f);
  d.orientation[3] = (char)(currRot.w * 127.f);

  *p = new DeltaPacket(d);
  return true;
}

bool NetworkObject::WriteFullPacket(GamePacket **p) {
  // Is it faster to create on stack and copy over, or new directly and indirect
  // every write?
  FullPacket f = FullPacket();

  f.objectID = networkID;
  f.fullState.position = object.GetTransform().GetPosition();
  f.fullState.orientation = object.GetTransform().GetOrientation();
  f.fullState.stateID = lastFullState.stateID++;

  *p = new FullPacket(f);
  return true;
}

NetworkState &NetworkObject::GetLatestNetworkState() { return lastFullState; }

bool NetworkObject::GetNetworkState(int stateID, NetworkState &state) {
  auto found = std::find_if(
      stateHistory.begin(), stateHistory.end(),
      [stateID](const NetworkState &s) { return s.stateID == stateID; });

  if (found != stateHistory.end()) {
    state = *found;
    return true;
  }

  return false;
}

void NetworkObject::UpdateStateHistory(int minID) {
  for (auto i = stateHistory.begin(); i != stateHistory.end();) {
    if (i->stateID < minID) {
      i = stateHistory.erase(i);
    } else {
      ++i;
    }
  }
}