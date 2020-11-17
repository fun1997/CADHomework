#include "Scene.h"
#include <QOpenGLDebugLogger>
#include "Primitive.h"
#include "PPolygon.h"
#include "PaintInformation.h"
#include "Light.h"
namespace vrt {
Scene::Scene(QWidget* parent)
	: QOpenGLWidget(parent)
{
	QSurfaceFormat format;
#ifdef _DEBUG
	//���������Ϣ
	format.setMajorVersion(3);
	format.setMinorVersion(3);
	format.setProfile(QSurfaceFormat::CoreProfile);
	format.setOption(QSurfaceFormat::DebugContext);
#endif
	//��������
	format.setSamples(32);
	this->setFormat(format);
}

QOpenGLDebugLogger* Scene::logger = nullptr;

Scene::~Scene()
{
	
}

void Scene::initializeGL()
{
	this->initializeOpenGLFunctions();

	this->glClearColor(0.7f, 0.7f, 0.8f, 1.0f);
	this->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//�����ʼ��
	camera.initialize(width(), height());

	//�������ʼ��
	axis.initialize();
	axis.updateViewMat(camera.getViewMatrixForAxis());
	axis.updateProjMat(camera.getProjMatrixForAxis());

	doPrimAdd();

#ifdef _DEBUG
	logger = new QOpenGLDebugLogger(this);
	logger->initialize();
#endif
}

void Scene::resizeGL(int w, int h)
{
	this->glViewport(0, 0, w, h);
	camera.initialize(w, h);
}

void Scene::paintGL()
{
	this->glEnable(GL_DEPTH_TEST);
	this->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	debugOpenGL();

	doPrimAdd();

	PaintInfomation info;
	info.projMat = camera.getProjMatrix();
	info.viewMat = camera.getViewMatrix();
	info.lights.push_back(std::shared_ptr<Light>(new ConstantLight(Vector3f(0.5, 0.5, 0.5), Vector3f(0.9, 0.9, 0.9), Vector3f(0.0f), Point3f(4000, 4000, -5000))));
	info.fillmode = FILL_WIREFRAME;
	info.lineWidth = 1.5f;
	info.width = width();
	info.height = height();

	for(const auto& prim : prims_) {
		prim.second->paint(&info);
	}

	//������
	this->glClear(GL_DEPTH_BUFFER_BIT);
	axis.updateViewMat(camera.getViewMatrixForAxis());
	axis.updateProjMat(camera.getProjMatrixForAxis());
	axis.paint();
}

void Scene::mouseMoveEvent(QMouseEvent *ev)
{
	//����Ҽ�������ת
	if (ev->buttons() & Qt::LeftButton)
	{
		QVector2D rot;
		QQuaternion rotation;
		rot.setX((ev->x() - mousePos.x()) / width());
		rot.setY((ev->y() - mousePos.y()) / height());

		//������ת
		rotation = QQuaternion::fromAxisAndAngle(currentUp, -200 * rot.x());
		camera.up = rotation.rotatedVector(currentUp);
		camera.position = rotation.rotatedVector(currentPosition);
		camera.front = rotation.rotatedVector(currentFront);

		//������ת
		rotation = QQuaternion::fromAxisAndAngle(QVector3D::crossProduct(-currentFront, currentUp).normalized(), 200 * rot.y());
		camera.up = rotation.rotatedVector(camera.up);
		camera.position = rotation.rotatedVector(camera.position);
		camera.front = rotation.rotatedVector(camera.front);

		update();
	}
	//����м�����ƽ��
	if (ev->buttons() & Qt::MidButton)
	{
		QVector2D move;
		move.setX((ev->x() - wheelPos.x()) / width());
		move.setY((ev->y() - wheelPos.y()) / height());

		camera.position = currentPosition + QVector3D::crossProduct(-camera.front, camera.up).normalized() * 3 * camera.zoom()*move.x()
			+ camera.up.normalized() * 3 * camera.zoom()*move.y();

		update();
	}
}

//���ֿ�������
void Scene::wheelEvent(QWheelEvent *ev) {
	camera.zoomControl(ev->delta());
	update();
}

void Scene::mousePressEvent(QMouseEvent *ev)
{
	//��ȡ���ʱ״̬
	if (ev->button() == Qt::LeftButton) {
		mousePos.setX(ev->x());
		mousePos.setY(ev->y());

		currentPosition = camera.position;
		currentFront = camera.front;
		currentUp = camera.up;
	}

	if (ev->button() == Qt::MidButton) {
		wheelPos.setX(ev->x());
		wheelPos.setY(ev->y());

		currentPosition = camera.position;
	}
}

void Scene::doPrimAdd()
{
	while (!primsToAdd.empty()) {
		const auto& prim = primsToAdd.back();
		prims_[prim->id()] = prim;
		prim->initialize();
		primsToAdd.pop_back();
	}
}

//�������ʴ�С
void Scene::zoomFit() {
	camera.fitZoom();
}

void Scene::debugOpenGL()
{
#ifdef _DEBUG
	const QList<QOpenGLDebugMessage> messages = logger->loggedMessages();
	for (const QOpenGLDebugMessage &message : messages)
		qDebug() << message;
#endif
}

void vrt::Scene::addPrimitive(std::shared_ptr<Primitive> prim)
{
	primsToAdd.push_back(prim);
}
}
