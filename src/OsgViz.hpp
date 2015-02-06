#ifndef _OSGVIZ_HPP_
#define _OSGVIZ_HPP_

#include <iostream>
#include <lib_manager/LibManager.hpp>
#include <osg/Group>
#include <OpenThreads/Thread>

#include "graphics/interfaces/GraphicsManagerInterface.h"
#include "plugins/OsgVizPlugin.h"

namespace osgviz {
	namespace graphics{
		class GraphicsManager;
	}
}

#include <stdio.h>

namespace osgviz
{

class OsgViz;

class FrameUpdateThread : public OpenThreads::Thread
	{
	    public:

	FrameUpdateThread(graphics::GraphicsManager* osgviz):
	        	osgviz(osgviz){
	        	running = false;
	        }

	        ~FrameUpdateThread()
	        {
	            if (isRunning())
	            {
	                cancel();
	                join();
	            }
	        }

	        int cancel()
	        {
	        	mutex.lock();
	        	running = false;
	        	mutex.unlock();
	            return 0;
	        }

	        void run();

	        void lock(){
	        	mutex.lock();
	        }
	        void unlock(){
	        	mutex.unlock();
	        }

	    private:
	        bool running;
	        graphics::GraphicsManager* osgviz;
	        OpenThreads::Mutex mutex;

	};


	class OsgViz: public lib_manager::LibInterface
	{

		public: 

	    CREATE_MODULE_INFO();

		static OsgViz* getInstance(int argc = 0,char** argv = NULL);

		OsgViz(lib_manager::LibManager * manager);

		OsgViz(int argc, char** argv);

		~OsgViz();


		virtual const std::string getLibName() const;

	    virtual int getLibVersion() const;



	    void init(int argc,char** argv);

	    /**
	     * starts a thread calling updateContent
	     */
	    void startThread();
	    void stopThread();

	    void lockThread();
	    void unlockThread();


		void updateContent();

		int createWindow(bool threaded = true);
		void destroyWindow(int id);

		inline GraphicsManagerInterface* getGraphicsManagerInterface(){
			return (GraphicsManagerInterface*)graphicsManager;
		}

		template <class VIZPLUGIN> VIZPLUGIN* getVisualizerPlugin(std::string classname){
			VIZPLUGIN* viz = (VIZPLUGIN*)getVizPlugin(classname,classname);
			viz->setRootNode(root);
			return viz;
		}

		template <class VIZPLUGIN> VIZPLUGIN* getDataPlugin(std::string classname){
			VIZPLUGIN* data = (VIZPLUGIN*)getVizPlugin(classname,classname);
			data->init(m_argc,m_argv);
			return data;
		}



		private:

		OsgVizPlugin* getVizPlugin(std::string path, std::string name);

		bool createdOwnManager;
		lib_manager::LibManager *libmanager;

		osg::ref_ptr<osg::Group> root;


		private:
		bool initialized;
		std::vector< OsgVizPlugin* >loadedPlugins;

		int m_argc;
		char** m_argv;


		FrameUpdateThread* thread;
		graphics::GraphicsManager* graphicsManager;

		//std::vector<osgViewer::Viewer *> viewers;

	};

} // end namespace osgviz

#endif // _DUMMYPROJECT_DUMMY_HPP_
