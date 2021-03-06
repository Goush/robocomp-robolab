/*
 *    Copyright (C) 2006-2010 by RoboLab - University of Extremadura
 *
 *    This file is part of RoboComp
 *
 *    RoboComp is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    RoboComp is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with RoboComp.  If not, see <http://www.gnu.org/licenses/>.
 */
/** \mainpage RoboComp::robexComp
 *
 * \section intro_sec Introduction
 *
 * The robexComp component implements functions of a differential mobile robex robot base. 
 *
 * \section interface_sec Interface
 *
 * robexComp provides methods for handling complement the robot, you can go at one speed, stop it, check the odometry and consult other states. To get information about a specific interface you should read the documentation of the associated component.
 *
 * \section install_sec Installation
 *
 * \subsection install1_ssec Software depencences
 * robexComp does not have specific software dependencies.
 *
 * \subsection install2_ssec Compile and install
 * cd Components/HAL/robexComp
 * <br>
 * cmake . && make
 * <br>
 * To install:
 * <br>
 * sudo make install
 *
 * \section guide_sec User guide
 *
 * \subsection config_ssec Configuration file
 *
 * <p>
 * The configuration file robexComp/etc/config you can select the device USB where connect the mobile robot base.
 * </p>
 *
 * \subsection execution_ssec Execution
 *
 * Just: "${PATH_TO_BINARY}/robexComp --Ice.Config=${PATH_TO_CONFIG_FILE}"
 *
 * \subsection running_ssec Once running
 *
 * .....
 *
 */



// QT includes
#include <QtCore>
#include <QtGui>

// ICE includes
#include <Ice/Ice.h>
#include <Ice/Application.h>

#include <rapplication/rapplication.h>

// View the config.h file for config options like
// QtGui, etc...
#include "config.h"

#include "worker.h"
#include "monitor.h"
#include "commonbehaviorI.h"
#include "differentialrobotI.h"
#include "joystickadapterI.h"
#include "genericbaseI.h"

using namespace std;
using namespace RoboCompCommonBehavior;
using namespace RoboCompDifferentialRobot;
using namespace RoboCompJoystickAdapter;
using namespace RoboCompGenericBase;


class DifferentialRobotComp : public RoboComp::Application 
{

private:
	// User private data here
	
	
	void initialize();

public:
	virtual int run(int, char*[]);
};

void DifferentialRobotComp::initialize()
{
	// Config file properties read example
	// configGetString( PROPERTY_NAME_1, property1_holder, PROPERTY_1_DEFAULT_VALUE );
	// configGetInt( PROPERTY_NAME_2, property1_holder, PROPERTY_2_DEFAULT_VALUE );
}

int DifferentialRobotComp::run(int argc, char* argv[])
{
	QCoreApplication a(argc, argv);  // NON-GUI application
	int status=EXIT_SUCCESS;

	// Remote server proxy access example
	// RemoteComponentPrx remotecomponent_proxy;


	string proxy;

	// User variables


	initialize();

	Worker *worker = new Worker();
	//Monitor thread
	Monitor *monitor = new Monitor(worker,communicator());
	QObject::connect(monitor,SIGNAL(kill()),&a,SLOT(quit()));
	QObject::connect(worker,SIGNAL(kill()),&a,SLOT(quit()));
	monitor->start();
	
	if ( !monitor->isRunning() )
	  return status;
	try
	{
		// Server adapter creation and publication
		Ice::ObjectAdapterPtr adapterBehavior = communicator()->createObjectAdapter("CommonBehavior");
		CommonBehaviorI *commonbehaviorI = new CommonBehaviorI(monitor );
		adapterBehavior->add(commonbehaviorI, communicator()->stringToIdentity("commonbehavior"));
		adapterBehavior->activate();
		
		Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("RobexComp");		
		DifferentialRobotI *differentialrobotI = new DifferentialRobotI(worker );
		adapter->add(differentialrobotI, communicator()->stringToIdentity("differentialrobot"));
		adapter->activate();
		
		Ice::ObjectAdapterPtr adapterJoy = communicator()->createObjectAdapter("JoystickAdapter");
		JoystickAdapterI *joystickadapterI = new JoystickAdapterI(worker);
		adapterJoy->add(joystickadapterI,communicator()->stringToIdentity("joystickadapter"));
		adapterJoy->activate();
		
		Ice::ObjectAdapterPtr adapterGenericBase = communicator()->createObjectAdapter("GenericBase");
		GenericBaseI *genericbaseI = new GenericBaseI(worker);
		adapterGenericBase->add(genericbaseI,communicator()->stringToIdentity("genericbase"));
		adapterGenericBase->activate();

		cout << SERVER_FULL_NAME " started" << endl;

		// User defined QtGui elements ( main window, dialogs, etc )

#ifdef USE_QTGUI
		//ignoreInterrupt(); // Uncomment if you want the component to ignore console SIGINT signal (ctrl+c).
		a.setQuitOnLastWindowClosed( true );
#endif
		// Run QT Application Event Loop
		a.exec();
		status = EXIT_SUCCESS;
	}
	catch(const Ice::Exception& ex)
	{
		status = EXIT_FAILURE;

		cout << "[" << PROGRAM_NAME << "]: Exception raised on main thread: " << endl;
		cout << ex;
	}

	return status;
}

int main(int argc, char* argv[])
{
	bool hasConfig = false;
	string arg;
	DifferentialRobotComp app;

	// Search in argument list for --Ice.Config= argument
	for (int i = 1; i < argc; ++i)
	{
		arg = argv[i];
		if ( arg.find ( "--Ice.Config=", 0 ) != string::npos )
			hasConfig = true;
	}

	if ( hasConfig )
		return app.main( argc, argv );
	else
		return app.main(argc, argv, "config"); // "config" is the default config file name
}
