#include "ofApp.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

template <class T>
void imgui_draw_tree_node(const char *name, bool isOpen, T f) {
	if (isOpen) {
		ImGui::SetNextTreeNodeOpen(true, ImGuiSetCond_Once);
	}
	if (ImGui::TreeNode(name)) {
		f();
		ImGui::TreePop();
	}
}

//--------------------------------------------------------------
void ofApp::setup(){
	_camera.setDistance(20);
	_camera.setNearClip(0.1);
	_camera.setFarClip(300.0);

	_imgui.setup();
}

//--------------------------------------------------------------
void ofApp::update() {
	double d = std::min(ofGetLastFrameTime(), 1.0 / 30.0);

	auto impluse = [](double x, double a) {
		double over_a = 1.0 / a;
		return over_a * x * glm::exp(1.0 - over_a * x);
	};
	auto near_more = [](double d, double s) {
		return glm::exp(-(d * d) / (s * s));
	};

	int N = 10;
	double delta = d / N;
	for (int i = 0; i < N; ++i) {
		// 差
		double d = _to_x - _x;

		// 比例制御(P)
		auto pv = d * _kP;

		// 距離の近いときの+速度
		auto nv = impluse(glm::abs(d), 0.5) * _approach;
		if (pv < 0.0) {
			nv = -nv;
		}

		auto v = pv + nv;

		// 速度制限
		v = std::min(v, (double)_vMax);
		v = std::max(v, (double)-_vMax);

		// 加速だけ制限をかけたい
		if (glm::abs(_v) < glm::abs(v)) {
			auto amax = _aMax * delta;
			double a = v - _v;
			a = std::min(a, amax);
			a = std::max(a, -amax);
			v = _v + a;
		}
		
		_v = v;
		_x += _v * delta;
	}

}

//--------------------------------------------------------------
void ofApp::draw(){
	ofClear(0);

	ofSetColor(255);
	_camera.begin();
	
	glPushMatrix();
	ofRotateZ(90);
	ofDrawGridPlane(1, 10);
	ofPopMatrix();

	ofDrawAxis(10);

	ofEnableDepthTest();
	
	// 位置場
	// Integrate[t + w*exp(-(x^2)/s^2), {x, 0, z}]
	auto position_field = [](double z, double s, double t, double w) {
		return 0.5 * glm::sqrt(glm::pi<double>()) * s * w * std::erf(z / s) + t * z;
	};

	// 回転場
	auto rotation_field = [](double z, double s) {
		auto v = glm::exp(-(z * z) / (s * s)) - 1.0;
		return 0.0 < z ? v : -v;
	};

	// ズーム場
	auto zoom_field = [](double z, double s) {
		return glm::exp(-(z * z) / (s * s));
	};

	// 奥に並ぶようなやつ
	// Integrate[erf(u*sqrt(pi) x), {x, 0, z}]
	//auto zf = [](double z, double u) {
	//	double sqPI = glm::sqrt(glm::pi<double>());
	//	double a = -glm::pi<double>() * u * z * std::erf(sqPI * u * z) - glm::exp(-glm::pi<double>() * u * u * z * z) + 1.0;
	//	double b = glm::pi<double>() * u;
	//	return -a / b;
	//};

	double step = 1.0;

	// double ofs = -(glm::sin(ofGetElapsedTimef() * 0.5) * 0.5 + 0.5) * 5.0;
	for (int i = 0; i < kSlideN; ++i) {
		double x = i * step - _x;
		double position = position_field(x, _positionRoughness, _edgeSlope, _centerArea);
		double rot = rotation_field(x, _rotationArea);
		double zoomValue = zoom_field(x, _zoomArea);
		// double ofz = - 1.0 * zf(x, 2.0);

		ofSetColor(i % 2 == 0 ? 128 : 200);
		ofPushMatrix();
		ofTranslate(position, 0, zoomValue * _zoom);
		ofRotateY(rot * _rotation);

		// 縦
		ofDrawRectangle(-0.5, -0.5, 1.0f, 1.0f);
		ofPopMatrix();
	}

	// テスト描画
	//ofSetColor(ofColor::red);
	//ofDrawSphere(_x, 1.0f, 0.5f);
	//ofSetColor(ofColor::green);
	//ofDrawSphere(_to_x, 1.0f, 0.4f);

	_camera.end();

	_imgui.begin();
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ofVec4f(0.0f, 0.2f, 0.2f, 0.8f));
	ImGui::SetNextWindowPos(ofVec2f(10, 30), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ofVec2f(500, ofGetHeight() * 0.8), ImGuiSetCond_Once);

	ImGui::Begin("Config");

	imgui_draw_tree_node("Cover Flow", true, [=]() {
		// 両端の間隔
		ImGui::SliderFloat("edgeSlope", &_edgeSlope, 0.0f, 1.0f);

		// 真ん中エリアの幅
		ImGui::SliderFloat("positionRoughness", &_positionRoughness, 0.0f, 4.0f);

		// 真ん中エリアの幅拡張
		ImGui::SliderFloat("centerArea", &_centerArea, 0.0f, 10.0f);

		// 回転エリアの幅
		ImGui::SliderFloat("rotationArea", &_rotationArea, 0.0f, 4.0f);

		// 回転量
		ImGui::SliderFloat("rotation", &_rotation, 0.0f, 90.0f);

		// z方向ズーム幅
		ImGui::SliderFloat("zoomArea", &_zoomArea, 0.0f, 4.0f);

		// z方向ズーム量
		ImGui::SliderFloat("zoom", &_zoom, 0.0f, 2.0f);
	});


	imgui_draw_tree_node("Movement", true, [=]() {
		// 差に比例する量 = ベース速度を決める
		ImGui::SliderFloat("kP", &_kP, 0.0f, 10.0f);

		// 最大速度制限
		ImGui::SliderFloat("vMax", &_vMax, 0.0f, 30.0f);

		// 最大加速度制限（上り）
		ImGui::SliderFloat("aMax", &_aMax, 0.0f, 200.0f);

		// 距離が近いときの追加加速具合制御 - 通常の比例制御だと減速が激しいため
		ImGui::SliderFloat("approach", &_approach, 0.0f, 5.0f);

		// 距離が近いときの追加加速を行う幅。おおむね0.5でいい気がする
		ImGui::SliderFloat("approachWide", &_approachWide, 0.0f, 5.0f);

		ImGui::Separator();

		// テスト用直接編集
		ImGui::SliderInt("selection", &_to_x, 0, kSlideN - 1);
	});

	ImGui::End();
	ImGui::PopStyleColor();

	_imgui.end();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if (key == OF_KEY_RIGHT) {
		_to_x += 1;
	}
	if (key == OF_KEY_LEFT) {
		_to_x -= 1;
	}

	_to_x = std::min(_to_x, kSlideN - 1);
	_to_x = std::max(_to_x, 0);
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
