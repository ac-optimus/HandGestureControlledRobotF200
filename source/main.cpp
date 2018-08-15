

#include <windows.h>
#include <iostream>
#include "pxcsensemanager.h"
#include "pxchandconfiguration.h"
#include"Serial.h"
#include "handanalysis_render.h"  //SDK provided utility class used for rendering face data (packaged in libpxcutils.lib)

using namespace std;

#define NUM_HANDS 1

int main(void) {
	tstring commPortName("COM7"); //put the port number to send the data serially to bluetooth.
	Serial serial(commPortName);//this opens the com port
	// error checking Status
	pxcStatus sts;
				  // initialize the util render 
	HandRender *renderer = new HandRender(L"Live_feed");//
	// create the PXCSenseManager instance
	PXCSenseManager *psm = 0;
	psm = PXCSenseManager::CreateInstance();//PXCsenseManager has static method CreateInstance() 
	if (!psm) {
		cout<<"Unable to create the PXCSenseManager"<<endl;
		return 1;
	}

	// select the depth stream of size 640x480
	psm->EnableStream(PXCCapture::STREAM_TYPE_DEPTH, 640, 480,60); //the third parameter is for fps

	// enable hand analysis in the multimodal pipeline
	sts = psm->EnableHand();//this fuction when called enables the hand tracking and returns the error status
	if (sts < PXC_STATUS_NO_ERROR) {
		cout<<"Unable to enable Hand Tracking"<<endl;
		return 2;
	}

	// retrieve hand module if ready - called in the setup stage before AcquireFrame
	PXCHandModule* handAnalyzer = psm->QueryHand();//get an instance of PXCHandModule   is PXCHandModule is a class of struct, can we create a pointer to a class??

	if (!psm) {
		cout<<"Unable to retrieve hand results"<<endl;
		return 3;
	}

	// initialize the PXCSenseManager pipeline
	if (psm->Init() < PXC_STATUS_NO_ERROR) return 4;
	//HERE THE CAMERA GETS STARTED,i.e lights gets on
	// retrieves an instance of the PXCHandData interface
	PXCHandData* outputData = handAnalyzer->CreateOutput();
	// retrieves an instance of the PXCHandData PXCHandConfiguration
	PXCHandConfiguration* config = handAnalyzer->CreateActiveConfiguration();//this funciton CreateActiveConfiguration is necessary to get an instance of PXCHnadConfigurION
	//MyHandler handler;//use this when event handler is used
	//config->SubscribeGesture(&handler);//this enables the event handler
	// enable or disable features in hand module
	config->EnableNormalizedJoints(true);
	config->EnableAlert(PXCHandData::AlertType::ALERT_HAND_DETECTED);
	config->EnableAlert(PXCHandData::AlertType::ALERT_HAND_NOT_DETECTED);
	config->EnableGesture(L"thumb_up");
	//config->EnableGesture(L"fist"); //fist is not working fine with thumb_up and thump_down
	//config->EnableGesture(L"click");
	config->EnableGesture(L"thumb_down");
	config->EnableGesture(L"v_sign");
	config->EnableGesture(L"swipe_left");
	config->EnableGesture(L"swipe_right");
	config->EnableGesture(L"two_fingers_pinch_open");
	config->EnableGesture(L"swipe_up");
	config->EnableGesture(L"swipe_up");
	//config->EnableGesture(L"spreadhand");
//	config->SubscribeGesture(&handler);
	//make the config changes effective
	config->ApplyChanges();

	// stream data
	int f = 0;
	PXCImage *depthIm = NULL; //init depth im
	while (psm->AcquireFrame(false) >= PXC_STATUS_NO_ERROR) {

		//do the event register thing here.
		//RUNS OVER THIS WHILE LOOP AGAIN AND AGAIN
		// increment frame counter since a frame is acquired
		
		//update the output data to the latest availible
		outputData->Update();

		// create data structs for storing data
		PXCHandData::GestureData gestureData;//handdata has the work to provide the data interface
		PXCHandData::JointData nodes[NUM_HANDS][PXCHandData::NUMBER_OF_JOINTS] = {};//if i am not wrong this is a two d empty array
		pxcCHAR gestures[NUM_HANDS][PXCHandData::MAX_NAME_SIZE] = {};//same here
		PXCHandData::BodySideType handSide[NUM_HANDS] = { PXCHandData::BODY_SIDE_UNKNOWN };//initially both th handSide element represent 0 i.e body side unknown
//		PXCHandData::FingerType fin;
//		PXCHandData::FingerData data;
//		PXCHandData::IHand::QueryFingerData;
		
		//we have just created the data structures above to store the data of hand tracking data
		// iterate through hands
		//pxcUID handID;   //i have commented this since it gave an error of unreferenced 
		pxcU16 numOfHands = outputData->QueryNumberOfHands();//checks for the numeber of hands visible
//get the joint data
		for (pxcU16 i = 0; i < numOfHands; i++)
		{
			// get hand joints by time of appearence
			PXCHandData::IHand* handData;
			if (outputData->QueryHandData(PXCHandData::ACCESS_ORDER_BY_TIME, i, handData) == PXC_STATUS_NO_ERROR)
			{
				// iterate through Joints and get joint data 
				for (int j = 0; j < PXCHandData::NUMBER_OF_JOINTS; j++)
				{
      					handData->QueryTrackedJoint((PXCHandData::JointType)j, nodes[i][j]);
				}
			}
			//if (handData->QueryFingerData(PXCHandData::FINGER_PINKY, data) == PXC_STATUS_NO_ERROR)
			//{
				//cout << "pinky,ponky!" << endl;
			//}
			//pxcUID l = handData->QueryUniqueId();
			//cout << l << endl;
			
		}//why is the above QueryUniqueId not working properly
		
		
		
		// iterate through fired gestures
		unsigned int k = outputData->QueryFiredGesturesNumber();//returns the number of gestures fired!
		for (unsigned int i = 0; i < k; i++)
		{
			// initialize data	
			wmemset(gestures[i], 0, sizeof(gestures[i]));
			handSide[i] = PXCHandData::BODY_SIDE_UNKNOWN;
			// get fired gesture data
			if (outputData->QueryFiredGestureData(i, gestureData) == PXC_STATUS_NO_ERROR)
			{
				f++;
//				cout << "value  " << f << endl;
				// get hand data related to fired gesture
				PXCHandData::IHand* handData;
				if (outputData->QueryHandDataById(gestureData.handId, handData) == PXC_STATUS_NO_ERROR)
				{
					// save gesture only if you know that its right/left hand
					if (!handData->QueryBodySide() == PXCHandData::BODY_SIDE_UNKNOWN)
					{
						wmemcpy_s(gestures[i], sizeof(gestureData.name), gestureData.name, sizeof(gestureData.name));
						handSide[i] = handData->QueryBodySide();//here is the side of body detected
						//cout << handSide[i] << endl;
					}
				}
			}
		}

		//cout << wcscmp(gestureData.name, L"thumb_up") << endl;
		/*if (!wcscmp(gestureData.name, L"thumb_up"))
			//if (outputData->IsGestureFired(L"thumb_up", gestureData))
		{
			serial.write("1");
			cout << "thumb_up" << endl;
		}
		else if (!wcscmp(gestureData.name,L"v_sign"))
		{
	//		serial.write("2");
			//cout << "why spreading hands baby!" << endl;
			cout << "v_sign" << endl;
		}
		else if (!wcscmp(gestureData.name, L"fist"))
		{
			cout << "fist" << endl;
		}
		else if (!wcscmp(gestureData.name, L"thumb_down"))
		{
			serial.write("3");
			//cout << "thumbs down huh!" << endl;
			cout << "thumb_down" << endl;
			
		}


		else if (!wcscmp(gestureData.name, L"two_fingers_pinch_open"))
		{
			cout << "two_finger_pinch_open" << endl;
			serial.write("8");
		}
		else if (!wcscmp(gestureData.name, L"click"))
		{
			cout << "click" << endl;
		//	serial.write("8");
		}*/
//		else if (!wcscmp(gestureData.name, L"swipe_right"))
	//	{
		//	cout << "swip_right" << endl;
			//serial.write("8");
	//	}
	//	else if (!wcscmp(gestureData.name, L"swipe_left"))
		//{
			//cout << "swipe_left" << endl;
		//}

		
		if (outputData->IsGestureFiredByHand(L"thumb_up", gestureData.handId, gestureData))
		{
			
			serial.write("1"); //move forward
			cout << "thumb_up" << endl;
			
		}

		else if (outputData->IsGestureFired(L"v_sign", gestureData))
		{
			serial.write("7"); //rotate clockwise
			cout << "v_sign" << endl;
		}
		else if (outputData->IsGestureFired(L"thumb_down", gestureData))
		{
			cout << "thumb_down" << endl;
			serial.write("0");//move back
		}
		else if (outputData->IsGestureFired(L"two_fingers_pinch_open", gestureData))
		{
			cout << "two_finger_pinch_open" << endl;
			serial.write("s"); //apply breaks
		}

		else if (outputData->IsGestureFired(L"swipe_right", gestureData))
		{
			serial.write("3"); // turn right 30 degree
			cout << "swip_right" << endl;
			
		}
		else if (outputData->IsGestureFired(L"swipe_left", gestureData))
		{
			serial.write("2"); // turn left 30 degrees
			cout << "swipe_left" << endl;
		}
		else if (outputData->IsGestureFired(L"swipe_down", gestureData))
		{
			cout << "swipe_down" << endl; //
		}
		else if (outputData->IsGestureFired(L"swipe_up", gestureData))
		{
			cout << "swipe_up" << endl;
		}
		// iterate through Alerts 
		//house keeping stuff to monitor alerts
		PXCHandData::AlertData alertData;
		
		for (int i = 0; i <outputData->QueryFiredAlertsNumber(); i++)
		{
			pxcStatus sts = outputData->QueryFiredAlertData(i, alertData);

			if (sts == PXC_STATUS_NO_ERROR)
			{
				// Display last alert - see AlertData::Label for all available alerts
				switch (alertData.label)
				{
				case PXCHandData::ALERT_HAND_DETECTED:
				{
					cout<<"Last Alert: Hand Detected"<<endl;
					break;
				}
				case PXCHandData::ALERT_HAND_NOT_DETECTED:
				{
					serial.write("8");
					cout<<"Last Alert: Hand Not Detected"<<endl;
					break;
				}
				}
			}
		}
		// retrieve all available image samples
		PXCCapture::Sample *sample = psm->QuerySample();

		// retrieve the image or frame by type
		depthIm = sample->depth;

		// render the frame
		if (!renderer->RenderFrame(depthIm, handAnalyzer, nodes, gestures, handSide))
			break;
		//HERE WE GET THE NEW FRAME FROM THE RENDER FUNCTION 
		// release or unlock the current frame to go fetch the next frame
		psm->ReleaseFrame();
		//HERE THE SAME FRAME IS RELEASED.
	}
	delete renderer;//whenever that is a new there is a delete for it.
					//delete the configuration
//	config->UnsubscribeGesture(&handler);
	config->Release();
	// delete the HandRender instance
	//	renderer->Release();

	// close the last opened stream and release any session and processing module instances
	psm->Release();

	return 0;
 }