#include "catch.hpp"
#include "DummyObjects.hpp"

namespace fast {

// Stream data only tests
TEST_CASE("Simple pipeline with stream", "[process_all_frames][ProcessObject][fast]") {
    Config::setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
    auto streamer = DummyStreamer::New();
    streamer->setSleepTime(10);
    streamer->setTotalFrames(20);

    auto po = DummyProcessObject::New();
    po->setInputConnection(streamer->getOutputPort());

    auto port = po->getOutputPort();
    port->setMaximumNumberOfFrames(1);

    bool lastFrame = false;
    int timestep = 0;
    while(!lastFrame) {
        po->update();
        auto image = port->getNextFrame<DummyDataObject>();
        lastFrame = image->isLastFrame();
        CHECK(image->getID() == timestep);
        timestep++;
    }
    CHECK(timestep == 20);
}

TEST_CASE("Two step pipeline with stream", "[two_step][process_all_frames][ProcessObject][fast]") {
    Config::setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
    auto streamer = DummyStreamer::New();
    streamer->setSleepTime(10);
    streamer->setTotalFrames(20);

    auto po1 = DummyProcessObject::New();
    po1->setInputConnection(streamer->getOutputPort());

    auto po2 = DummyProcessObject::New();
    po2->setInputConnection(po1->getOutputPort());

    auto port = po2->getOutputPort();

    int timestep = 0;
    bool lastFrame = false;
    while(!lastFrame) {
        po2->update();
        auto image = port->getNextFrame<DummyDataObject>();
        lastFrame = image->isLastFrame();
        CHECK(image->getID() == timestep);
        timestep++;
    }
    CHECK(timestep == 20);
}

TEST_CASE("Simple pipeline with stream NEWEST_FRAME_ONLY", "[ProcessObject][fast][newest_frame_only]") {
    Config::setStreamingMode(STREAMING_MODE_NEWEST_FRAME_ONLY);
    auto streamer = DummyStreamer::New();
    streamer->setSleepTime(100);
    streamer->setTotalFrames(20);

    auto po = DummyProcessObject::New();
    po->setInputConnection(streamer->getOutputPort());

    auto port = po->getOutputPort();

    int timestep = 0;
    int previousID = -1;
    bool lastFrame = false;
    while(!lastFrame) {
        po->update();
        timestep++;
        auto image = port->getNextFrame<DummyDataObject>();
        lastFrame = image->isLastFrame();
        CHECK(image->getID() != previousID); // TODO Should never get the same frame, or??
        previousID = image->getID();
    }
    Config::setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
}

TEST_CASE("Two step pipeline with stream NEWEST_FRAME_ONLY", "[two_step][ProcessObject][fast][newest_frame_only]") {
    Config::setStreamingMode(STREAMING_MODE_NEWEST_FRAME_ONLY);
    auto streamer = DummyStreamer::New();
    streamer->setSleepTime(100);
    streamer->setTotalFrames(20);

    auto po1 = DummyProcessObject::New();
    po1->setInputConnection(streamer->getOutputPort());

    auto po2 = DummyProcessObject::New();
    po2->setInputConnection(po1->getOutputPort());

    auto port = po2->getOutputPort();

    int timestep = 0;
    int previousID = -1;
    bool lastFrame = false;
    while(!lastFrame) {
        po2->update();
        timestep++;
        auto image = port->getNextFrame<DummyDataObject>();
        lastFrame = image->isLastFrame();
        CHECK(image->getID() != previousID); // Should never get the same frame
        previousID = image->getID();
    }
    Config::setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
}

/*
TEST_CASE("Simple pipeline with stream STORE_ALL", "[ProcessObject][fast]") {
    int frames = 20;
    auto streamer = DummyStreamer::New();
    streamer->setSleepTime(10);
    streamer->setTotalFrames(frames);

    auto po = DummyProcessObject::New();
    po->setInputConnection(streamer->getOutputPort());

    auto port = po->getOutputPort();

    //for(int i = 0; i < 3; i++) { // Loop over data stream 3 times
    //    std::cout << "NEW ROUND IN STORE ALL TEST" << std::endl;
        int timestep = 0;
        int previousID = -1;
        while(timestep < frames) {
            po->update();
            timestep++;
            auto image = port->getNextFrame<DummyDataObject>();
            CHECK(image->getID() != previousID); // Should never get the same frame
            previousID = image->getID();
        }
    //}
}

TEST_CASE("Two step pipeline with stream STORE_ALL", "[two_step][ProcessObject][fast]") {
    int frames = 20;
    auto streamer = DummyStreamer::New();
    streamer->setSleepTime(10);
    streamer->setTotalFrames(frames);

    auto po1 = DummyProcessObject::New();
    po1->setInputConnection(streamer->getOutputPort());

    auto po2 = DummyProcessObject::New();
    po2->setInputConnection(po1->getOutputPort());

    auto port = po2->getOutputPort();

    //for(int i = 0; i < 3; i++) { // Loop over data stream 3 times
    //    std::cout << "NEW ROUND IN STORE ALL TEST" << std::endl;
        int timestep = 0;
        int previousID = -1;
        while(timestep < frames) {
            po2->update();
            timestep++;
            auto image = port->getNextFrame<DummyDataObject>();
            CHECK(image->getID() != previousID); // Should never get the same frame
            previousID = image->getID();
        }
    //}
}
 */

// Static data only tests
TEST_CASE("Simple pipeline with static data", "[process_all_frames][ProcessObject][fast]") {
    Config::setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
    auto importer = DummyImporter::New();

    auto po1 = DummyProcessObject::New();
    po1->setInputConnection(importer->getOutputPort());

    auto port = po1->getOutputPort();

    int timestep = 0;
    po1->update();
    CHECK(po1->hasExecuted());
    po1->setHasExecuted(false);

    while(timestep < 3) {
        timestep++;
        po1->update();
        CHECK_FALSE(po1->hasExecuted()); // PO1 should not re execute
        po1->setHasExecuted(false);
        auto image = port->getNextFrame<DummyDataObject>();
        CHECK(image->getID() == 0); // Should always get the same frame
    }
    std::cout << "SETTING importer to modified" << std::endl;
    importer->setModified();
    po1->update();
    CHECK(po1->hasExecuted()); // PO should execute after importer has been modified
    po1->setHasExecuted(false);

    while(timestep < 6) {
        timestep++;
        po1->update();
        CHECK_FALSE(po1->hasExecuted()); // PO1 should not re execute
        po1->setHasExecuted(false);
        auto image = port->getNextFrame<DummyDataObject>();
        CHECK(image->getID() == 1); // Should always get the same frame
    }
    Config::setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
}

TEST_CASE("Simple pipeline with static data NEWEST_FRAME", "[ProcessObject][fast][newest_frame_only]") {
    Config::setStreamingMode(STREAMING_MODE_NEWEST_FRAME_ONLY);
    auto importer = DummyImporter::New();

    auto po1 = DummyProcessObject::New();
    po1->setInputConnection(importer->getOutputPort());

    auto port = po1->getOutputPort();

    int timestep = 0;
    while(timestep < 3) {
        po1->update();
        timestep++;
        auto image = port->getNextFrame<DummyDataObject>();
        CHECK(image->getID() == 0); // Should always get the same frame
    }

    importer->setModified();
    //timestep = 0;
    while(timestep < 6) {
        po1->update();
        timestep++;
        auto image = port->getNextFrame<DummyDataObject>();
        CHECK(image->getID() == 1); // Should always get the same frame
    }
    Config::setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
}

/*
TEST_CASE("Simple pipeline with static data, STORE_ALL", "[ProcessObject][fast]") {
    auto importer = DummyImporter::New();

    auto po1 = DummyProcessObject::New();
    po1->setInputConnection(importer->getOutputPort());

    auto port = po1->getOutputPort();

    int timestep = 0;
    while(timestep < 3) {
        po1->update();
        timestep++;
        auto image = port->getNextFrame<DummyDataObject>();
        CHECK(image->getID() == 0); // Should always get the same frame
    }

    importer->setModified();
    //timestep = 0;
    while(timestep < 6) {
        po1->update();
        timestep++;
        auto image = port->getNextFrame<DummyDataObject>();
        CHECK(image->getID() == 1); // Should always get the same frame
    }
}
*/

// Static + stream data tests
TEST_CASE("Simple pipeline with static and stream data PROCESS_ALL", "[process_all_frames][static_and_stream][ProcessObject][fast]") {
    Config::setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
    int frames = 10;
    auto streamer = DummyStreamer::New();
    streamer->setSleepTime(10);
    streamer->setTotalFrames(frames);

    auto importer = DummyImporter::New();

    auto po1 = DummyProcessObject2::New();
    po1->setInputConnection(0, streamer->getOutputPort());
    po1->setInputConnection(1, importer->getOutputPort());

    auto port = po1->getOutputPort();

    int timestep = 0;
    while(timestep < frames / 2) {
        po1->update();
        auto image = port->getNextFrame<DummyDataObject>();
        CHECK(image->getID() == timestep);
        CHECK(po1->getStaticDataID() == 0);
        timestep++;
    }

    importer->setModified();
    while(timestep < frames) {
        po1->update();
        auto image = port->getNextFrame<DummyDataObject>();
        CHECK(image->getID() == timestep);
        CHECK(po1->getStaticDataID() == 1);
        timestep++;
    }
    Config::setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
}

TEST_CASE("Static data with multiple receiver POs PROCESS_ALL", "[process_all_frames][ProcessObject][fast]") {
    Config::setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
    const int frames = 4;
    auto importer = DummyImporter::New();

    auto po1 = DummyProcessObject::New();
    po1->setInputConnection(importer->getOutputPort());
    auto po2 = DummyProcessObject::New();
    po2->setInputConnection(importer->getOutputPort());
    auto po3 = DummyProcessObject::New();
    po3->setInputConnection(po2->getOutputPort());

    auto port1 = po1->getOutputPort();
    auto port2 = po3->getOutputPort();

    int timestep = 0;
    while(timestep < frames) {
        // Update both branches
        po1->update();
        po3->update();

        auto image1 = port1->getNextFrame<DummyDataObject>();
        auto image2 = port2->getNextFrame<DummyDataObject>();

        CHECK(image1->getID() == 0);
        CHECK(image2->getID() == 0);
        timestep++;
    }
    Config::setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
}

TEST_CASE("Static data with multiple receiver POs NEWEST_FRAME", "[ProcessObject][fast]") {
    Config::setStreamingMode(STREAMING_MODE_NEWEST_FRAME_ONLY);
    const int frames = 4;
    auto importer = DummyImporter::New();

    auto po1 = DummyProcessObject::New();
    po1->setInputConnection(importer->getOutputPort());
    auto po2 = DummyProcessObject::New();
    po2->setInputConnection(importer->getOutputPort());
    auto po3 = DummyProcessObject::New();
    po3->setInputConnection(po2->getOutputPort());

    auto port1 = po1->getOutputPort();
    auto port2 = po3->getOutputPort();

    int timestep = 0;
    while(timestep < frames) {
        // Update both branches
        po1->update();
        po3->update();

        auto image1 = port1->getNextFrame<DummyDataObject>();
        auto image2 = port2->getNextFrame<DummyDataObject>();

        CHECK(image1->getID() == 0);
        CHECK(image2->getID() == 0);
        timestep++;
    }
    Config::setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
}

/*
TEST_CASE("Static data with multiple receiver POs STORE_ALL", "[ProcessObject][fast][asdasd2]") {
    const int frames = 4;
    auto importer = DummyImporter::New();

    auto po1 = DummyProcessObject::New();
    po1->setInputConnection(importer->getOutputPort());
    auto po2 = DummyProcessObject::New();
    po2->setInputConnection(importer->getOutputPort());
    auto po3 = DummyProcessObject::New();
    po3->setInputConnection(po2->getOutputPort());

    auto port1 = po1->getOutputPort();
    auto port2 = po3->getOutputPort();

    int timestep = 0;
    while(timestep < frames) {
        std::cout << "TIMESTEP " << timestep << std::endl;
        // Update both branches
        po1->update();
        po3->update();

        auto image1 = port1->getNextFrame<DummyDataObject>();
        auto image2 = port2->getNextFrame<DummyDataObject>();

        CHECK(image1->getID() == 0);
        CHECK(image2->getID() == 0);
        timestep++;
    }
}
 */

TEST_CASE("Stream with multiple receiver POs PROCESS_ALL", "[process_all_frames][ProcessObject][fast]") {
    Config::setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
    const int frames = 20;
    auto streamer = DummyStreamer::New();
    streamer->setSleepTime(10);
    streamer->setTotalFrames(frames);

    auto po1 = DummyProcessObject::New();
    po1->setInputConnection(streamer->getOutputPort());
    auto po2 = DummyProcessObject::New();
    po2->setInputConnection(streamer->getOutputPort());
    auto po3 = DummyProcessObject::New();
    po3->setInputConnection(po2->getOutputPort());

    auto port1 = po1->getOutputPort();
    auto port2 = po3->getOutputPort();

    int timestep = 0;
    while(timestep < frames) {
        // Update both branches
        po1->update();
        po3->update();

        auto image1 = port1->getNextFrame<DummyDataObject>();
        auto image2 = port2->getNextFrame<DummyDataObject>();

        CHECK(image1->getID() == timestep);
        CHECK(image2->getID() == timestep);
        timestep++;
    }
    Config::setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
}


TEST_CASE("Stream with multiple receiver POs, NEWEST_FRAME", "[ProcessObject][fast]") {
    Config::setStreamingMode(STREAMING_MODE_NEWEST_FRAME_ONLY);
    const int frames = 20;
    auto streamer = DummyStreamer::New();
    streamer->setSleepTime(10);
    streamer->setTotalFrames(frames);

    auto po1 = DummyProcessObject::New();
    po1->setInputConnection(streamer->getOutputPort());
    auto po2 = DummyProcessObject::New();
    po2->setInputConnection(streamer->getOutputPort());
    auto po3 = DummyProcessObject::New();
    po3->setInputConnection(po2->getOutputPort());

    auto port1 = po1->getOutputPort();
    auto port2 = po3->getOutputPort();

    int timestep = 0;
    int previousID1 = -1;
    int previousID2 = -1;
    while(timestep < frames) {
        // Update both branches
        po1->update();
        po3->update();

        auto image1 = port1->getNextFrame<DummyDataObject>();
        auto image2 = port2->getNextFrame<DummyDataObject>();

        // Should never process the same image twice
        CHECK(image1->getID() != previousID1);
        CHECK(image2->getID() != previousID2);
        previousID1 = image1->getID();
        previousID2 = image2->getID();
        timestep++;
    }
    Config::setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
}

/*
TEST_CASE("Stream with multiple receiver POs STORE_ALL", "[ProcessObject][fast]") {
    const int frames = 20;
    auto streamer = DummyStreamer::New();
    streamer->setSleepTime(10);
    streamer->setTotalFrames(frames);

    auto po1 = DummyProcessObject::New();
    po1->setInputConnection(streamer->getOutputPort());
    auto po2 = DummyProcessObject::New();
    po2->setInputConnection(streamer->getOutputPort());
    auto po3 = DummyProcessObject::New();
    po3->setInputConnection(po2->getOutputPort());

    auto port1 = po1->getOutputPort();
    auto port2 = po3->getOutputPort();

    //for(int i = 0; i < 3; i++) {
        int timestep = 0;
        while(timestep < frames) {
            // Update both branches
            po1->update();
            po3->update();

            auto image1 = port1->getNextFrame<DummyDataObject>();
            auto image2 = port2->getNextFrame<DummyDataObject>();

            // Should never process the same image twice
            CHECK(image1->getID() == timestep);
            CHECK(image2->getID() == timestep);
            timestep++;
        }
    //}
}
 */

// Static data only tests using setInputData
TEST_CASE("Simple pipeline with static data using setInputData", "[process_all_frames][ProcessObject][fast]") {
    Config::setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
    auto image = DummyDataObject::New();
    image->create(0);

    auto po1 = DummyProcessObject::New();
    po1->setInputData(image);

    auto port = po1->getOutputPort();

    int timestep = 0;
    while(timestep < 3) {
        po1->update();
        timestep++;
        auto image = port->getNextFrame<DummyDataObject>();
        CHECK(image->getID() == 0); // Should always get the same frame
    }
    Config::setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
}

TEST_CASE("Simple pipeline with static data using setInputData NEWEST_FRAME", "[ProcessObject][fast]") {
    Config::setStreamingMode(STREAMING_MODE_NEWEST_FRAME_ONLY);
    auto image = DummyDataObject::New();
    image->create(0);

    auto po1 = DummyProcessObject::New();
    po1->setInputData(image);

    auto port = po1->getOutputPort();

    int timestep = 0;
    while(timestep < 3) {
        po1->update();
        timestep++;
        auto image = port->getNextFrame<DummyDataObject>();
        CHECK(image->getID() == 0); // Should always get the same frame
    }
    Config::setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
}

/*
TEST_CASE("Simple pipeline with static data using setInputData STORE_ALL", "[ProcessObject][fast]") {
    auto image = DummyDataObject::New();
    image->create(0);

    auto po1 = DummyProcessObject::New();
    po1->setInputData(image);

    auto port = po1->getOutputPort();

    int timestep = 0;
    while(timestep < 3) {
        po1->update();
        timestep++;
        auto image = port->getNextFrame<DummyDataObject>();
        CHECK(image->getID() == 0); // Should always get the same frame
    }
}
 */

TEST_CASE("Missing input throws exception on execute", "[ProcessObject][fast]") {
    auto po = DummyProcessObject::New();
    po->setIsModified();
    CHECK_THROWS(po->update());
}

TEST_CASE("Trying to use non existent input and output ports throws exception", "[ProcessObject][fast]") {
    auto po = DummyProcessObject::New();
    CHECK_THROWS(po->getOutputPort(1));

    auto po2 = DummyProcessObject2::New();
    CHECK_THROWS(po->setInputConnection(1, po2->getOutputPort()));

    auto image = DummyDataObject::New();
    CHECK_THROWS(po->setInputData(1, image));
}

TEST_CASE("Trying to set input connection to self throws exception", "[ProcessObject][fast]") {
    auto po = DummyProcessObject::New();
    CHECK_THROWS(po->setInputConnection(po->getOutputPort()));
}

}
