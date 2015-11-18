#include "DataObject.hpp"
#include "ProcessObject.hpp"

namespace fast {

DataObject::DataObject() : mTimestampModified(0), mIsDynamicData(false) {
    // Create scene graph node and attach it to a new root
    SceneGraphNode::pointer newRootNode = SceneGraphNode::New();
    SceneGraphNode::pointer node = SceneGraphNode::New();
    node->setParent(newRootNode);
    mSceneGraphNode = node;
}

SceneGraphNode::pointer DataObject::getSceneGraphNode() const {
    return mSceneGraphNode;
}

bool DataObject::isDynamicData() {
    return mIsDynamicData;
}

unsigned long DataObject::getTimestamp() {
    return mTimestampModified;
}

void DataObject::updateModifiedTimestamp() {
    mTimestampModified++;
}

void DataObject::retain(ExecutionDevice::pointer device) {
    if(mReferenceCount.count(device) == 0) {
        mReferenceCount[device] = 1;
    } else {
        mReferenceCount[device]++;
    }
}

void DataObject::release(ExecutionDevice::pointer device) {
    if(mReferenceCount.count(device) == 0)
        throw Exception("Trying to release data on a device that has not retained the data.");
    mReferenceCount[device]--;

    // TODO: if there is data left for a data object it must be marked as modified somehow so that it will call update on itself
    // when someone request access on it
    if(mReferenceCount[device] == 0) {
        this->free(device);
    }
}

BoundingBox DataObject::getBoundingBox() const {
    return mBoundingBox;
}

BoundingBox DataObject::getTransformedBoundingBox() const {
    LinearTransformation T = SceneGraph::getLinearTransformationFromData(DataObject::pointer(mPtr.lock()));

    return getBoundingBox().getTransformedBoundingBox(T);
}


void DataObject::setStreamer(Streamer::pointer streamer) {
    mStreamer = streamer;
}

Streamer::pointer DataObject::getStreamer() {
    return mStreamer.lock();
}

} // end namespace fast