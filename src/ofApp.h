#pragma once

#include "ofMain.h"
#include "ofxImGui.h"

static int kSlideN = 15;

class ofApp : public ofBaseApp{

public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
		
	ofEasyCam _camera;
	ofxImGui _imgui;
	
	// coverflow
	float _edgeSlope = 0.3;
	float _positionRoughness = 0.7;
	float _centerArea = 1.1;
	float _rotationArea = 0.7;
	float _rotation = 70.0;
	float _zoomArea = 0.7;
	float _zoom = 0.5;

	// move
	float _kP = 5.0f;
	float _vMax = 15.0f;
	float _aMax = 40.0f;
	float _approach = 1.0f;
	float _approachWide = 0.5f;

	// 現在座標
	double _x = 0.0;
	// 現在速度
	double _v = 0.0;
	// selection
	int _to_x = 0;
};
