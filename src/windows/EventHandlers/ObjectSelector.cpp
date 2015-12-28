/*
 * ObjectSelector.cpp
 *
 *  Created on: 09.03.2015
 *      Author: planthaber
 */

#include "ObjectSelector.h"

#include "../../OsgViz.hpp"
#include "../../interfaces/Clickable.h"
#include "../SuperView.h"

#include <osgUtil/LineSegmentIntersector>
#include <iostream>


namespace osgviz {

ObjectSelector::ObjectSelector(osgviz::SuperView *view):osgGA::GUIEventHandler(),view(view) {
	draggedObject= NULL;

}

ObjectSelector::~ObjectSelector() {
	// TODO Auto-generated destructor stub
}

std::vector<ObjectSelector::IntersectionResult> ObjectSelector::getIntersections(const osgGA::GUIEventAdapter& ea){

	osg::ref_ptr<osgUtil::LineSegmentIntersector> ray = new osgUtil::LineSegmentIntersector(osgUtil::Intersector::PROJECTION, ea.getXnormalized(), ea.getYnormalized());
	osgUtil::IntersectionVisitor visitor(ray);

	view->getCamera()->accept(visitor);

	//osgUtil::LineSegmentIntersector::Intersections intersection = ray->getIntersections();
	osgUtil::LineSegmentIntersector::Intersection intersection = ray->getFirstIntersection();

	std::vector<IntersectionResult> results;
	IntersectionResult result;
	//results.reserve(intersections.size());
	result.c = osg::Vec2d(ea.getX(),ea.getY());

	if( ray->containsIntersections()){

//		for (osgUtil::LineSegmentIntersector::Intersections::iterator intersection = intersections.begin();intersection!=intersections.end();intersection++){

			result.p = intersection.getLocalIntersectPoint();
			result.w = intersection.getWorldIntersectPoint();

			for (osg::NodePath::const_iterator node = intersection.nodePath.begin(); node != intersection.nodePath.end(); node++){
				Clickable* obj = dynamic_cast<Clickable*>(*node);
				if (obj){
					result.clickable = obj;
					results.push_back(result);
				}
			}
//		}
	}

	return results;
}


bool ObjectSelector::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa){
    thisEvent = ea.getEventType();

    //only evaluate following events
    if (	   thisEvent == osgGA::GUIEventAdapter::PUSH
            || thisEvent == osgGA::GUIEventAdapter::RELEASE
            || thisEvent == osgGA::GUIEventAdapter::DRAG
    ){


        //save the buttonmask (not available in release event)
        if (thisEvent == osgGA::GUIEventAdapter::PUSH){
            pushedButtonsMask = ea.getButtonMask();
        }

        //normal click
        if(thisEvent & osgGA::GUIEventAdapter::RELEASE && lastEvent & osgGA::GUIEventAdapter::PUSH){
            lastEvent = thisEvent;

            std::vector<IntersectionResult> intersections = getIntersections(ea);

            if (!intersections.empty()){
                for (std::vector<IntersectionResult>::iterator inter = intersections.begin(); inter != intersections.end(); inter++){
                        if (inter->clickable->clicked(pushedButtonsMask,inter->c,inter->w,inter->p,inter->clickable,view)){
                            return true;
                        }
                }
            }
        }

        //drag

        if (thisEvent & osgGA::GUIEventAdapter::DRAG && lastEvent & osgGA::GUIEventAdapter::PUSH){
            //get the first Clickabe Object in path
            draggedObject = NULL;

            std::vector<IntersectionResult> intersections = getIntersections(ea);

            if (!intersections.empty()){
                for (std::vector<IntersectionResult>::iterator inter = intersections.begin(); inter != intersections.end(); inter++){
                    if (inter->clickable->dragged(pushedButtonsMask,inter->c,inter->w,inter->p,inter->clickable,view)){
                        //there is a receiving obj,
                        view->disableCameraControl();
                        draggedObject = inter->clickable;
                        lastEvent = thisEvent;
                        return true;
                    }
                }
            }

        }

        if (thisEvent & osgGA::GUIEventAdapter::DRAG){
            //don't need checks, the initial object is used
            lastEvent = thisEvent;
            if (draggedObject){

                std::vector<IntersectionResult> intersections = getIntersections(ea);

                for (std::vector<IntersectionResult>::iterator inter = intersections.begin(); inter != intersections.end(); inter++){
                    if (inter->clickable == draggedObject){
                        return draggedObject->dragged(pushedButtonsMask,inter->c,inter->w,inter->p,draggedObject,view);
                    }
                }
            }
            return false;
        }

        if (thisEvent & osgGA::GUIEventAdapter::RELEASE && lastEvent & osgGA::GUIEventAdapter::DRAG){
            view->enableCameraControl();
            lastEvent = thisEvent;
            if (draggedObject){
                //dragging stopped call calback with 0 as buttom mask
                int buttons = 0;
                bool res = draggedObject->dragged(buttons,osg::Vec2d(0,0),osg::Vec3(0,0,0),osg::Vec3(0,0,0),draggedObject,view);
                draggedObject = NULL;
                return res;
            }
        }



        lastEvent = thisEvent;

        if (draggedObject){
            //no intersection, but still dragging
            return true;
        }
        return false;
    }
    return false;
}

} /* namespace osgviz */