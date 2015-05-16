//******************************************************************
//
// Copyright 2014 Samsung Electronics All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include "OCPlatform.h"
#include "OCApi.h"

#include <functional>
#include <pthread.h>
#include <iostream>

using namespace std;
using namespace OC;

namespace PH = std::placeholders;

OCResourceHandle resourceHandle;
std::vector< OCResourceHandle > resourceHandleVector;

void foundResource(std::shared_ptr< OCResource > resource)
{

    std::string resourceURI;
    std::string hostAddress;

    try
    {
        cout << "FOUND RESOURCE" << endl;

        if (resource)
        {
            resourceURI = resource->uri();
            hostAddress = resource->host();

            cout << "\tResource URI : " << resourceURI << endl;
            cout << "\tResource Host : " << hostAddress << endl;
            cout << "\tResource Interfaces : " << resource->getResourceInterfaces().front() << endl;
            cout << "\tResource Type : " << resource->getResourceTypes().front() << endl;
            if (resourceURI == "/a/light" || resourceURI == "/a/fan")
            {
                OCResourceHandle foundResourceHandle;
                OCStackResult result = OCPlatform::registerResource(foundResourceHandle, resource);
                cout << "\tresource registed!" << endl;
                if (result == OC_STACK_OK)
                {
                    OCPlatform::bindResource(resourceHandle, foundResourceHandle);
                    resourceHandleVector.push_back(foundResourceHandle);
                }
                else
                {
                    cout << "\tresource Error!" << endl;
                }
            }
        }
    }
    catch (std::exception& e)
    {
        std::cout << "Exception in foundResource:"<< e.what() << std::endl;
    }

}

int main(int argc, char* argv[])
{
    ostringstream requestURI;

    OCConnectivityType connectivityType = OC_WIFI;

    if(argc == 2)
    {
        try
        {
            std::size_t inputValLen;
            int optionSelected = stoi(argv[1], &inputValLen);

            if(inputValLen == strlen(argv[1]))
            {
                if(optionSelected == 0)
                {
                    connectivityType = OC_ETHERNET;
                }
                else if(optionSelected == 1)
                {
                    connectivityType = OC_WIFI;
                }
                else
                {
                    std::cout << "Invalid connectivity type selected. Using default WIFI"
                        << std::endl;
                }
            }
            else
            {
                std::cout << "Invalid connectivity type selected. Using default WIFI" << std::endl;
            }
        }
        catch(exception&)
        {
            std::cout << "Invalid input argument. Using WIFI as connectivity type" << std::endl;
        }
    }
    else
    {
        std::cout<<"Usage: groupclient <ConnectivityType(0|1)>\n";
        std::cout<<"ConnectivityType: Default WIFI\n";
        std::cout<<"ConnectivityType 0: ETHERNET\n";
        std::cout<<"ConnectivityType 1: WIFI\n";
    }

    PlatformConfig config
    { OC::ServiceType::InProc, ModeType::Both, "0.0.0.0", 0, OC::QualityOfService::LowQos };

    try
    {
        string resourceURI = "/core/a/collection";
        string resourceTypeName = "a.collection";
        string resourceInterface = BATCH_INTERFACE;
        OCPlatform::Configure(config);

        // EntityHandler cb = std::bind(, PH::_1, PH::_2);

        OCPlatform::registerResource(resourceHandle, resourceURI, resourceTypeName,
                resourceInterface,
                NULL,
                //&entityHandler, // entityHandler
                OC_DISCOVERABLE);

        cout << "registerResource is called." << endl;

        requestURI << OC_MULTICAST_DISCOVERY_URI << "?rt=core.light";

        OCPlatform::findResource("", requestURI.str(),
                                 OC_ALL, &foundResource);

        OCPlatform::bindInterfaceToResource(resourceHandle, GROUP_INTERFACE);
        OCPlatform::bindInterfaceToResource(resourceHandle, DEFAULT_INTERFACE);

        int selectedMenu;
        bool isRun = true;
        while (isRun)
        {
            cout << endl << "0 :: Quit 1 :: UNREGISTER RESOURCES\n" << endl;
            std::cin >> selectedMenu;

            switch(selectedMenu)
            {
            case 0:
                isRun = false;
                break;
            case 1:
                std::cout << "Unregistering resources" << std::endl;
                for (unsigned int i = 0; i < resourceHandleVector.size(); ++i)
                {
                    OCPlatform::unregisterResource(resourceHandleVector.at(i));
                }
                break;
            default:
                cout << "Invalid option" << endl;
            }

        }
    }
    catch (OCException& e)
    {
        oclog() << "Exception in main: "<< e.what();
    }

    return 0;
}
