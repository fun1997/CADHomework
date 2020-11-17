#include "PPolygon.h"
#include "glut.h"
#include "Scene.h"
#include "PaintInformation.h"
#include "utilities.h"
#include "Light.h"
namespace vrt {
	// #TODO ʹ�ü�����ɫ������������ʵ�ֱ߿����ɫ
	const char *vertexShaderSource_Mesh =
		"#version 330 core\n"
		"layout(location = 0) in vec3 aPos;\n"
		"uniform mat4 modelMat;\n"
		"uniform mat4 viewMat;\n"
		"uniform mat4 projMat;\n"
		"void main(){\n"
		"  gl_Position = projMat * viewMat * modelMat * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
		"}\n\0";

	const char *fragmentShaderSource_Mesh =
		"#version 330 core\n"
		"uniform vec4 ourColor;\n"
		"out vec4 FragColor;\n"
		"void main(){\n"
		"   FragColor = ourColor;\n"
		"}\n\0";

	std::string vertexShaderSource_LightedMesh =
		"#version 330 core\n"
		"layout(location = 0) in vec3 aPos;\n"
		"uniform vec3 normal;\n"
		"uniform vec4 ourColor;\n"
		"out vec4 aColor;"
		"uniform mat4 modelMat;\n"
		"uniform mat4 viewMat;\n"
		"uniform mat4 projMat;\n"
		"struct Light{\n"
		"vec3 pos; vec3 ambient; vec3 diffuse; vec3 specular;};\n"
		"uniform Light lights[" + std::to_string(MAX_LIGHT_COUNT) + "];\n"
		"uniform int lightCount;\n"
		"void main(){\n"
		"   gl_Position = projMat * viewMat * modelMat * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
		"   vec3 FragPos = vec3(viewMat * modelMat * vec4(aPos, 1.0));\n"
		"	vec4 tempColor;\n"
		"   if(normal.z < 0) tempColor = vec4(0.1,0.1,0.1,1.0);\n"  //������ʾȫ��
		"	else tempColor = ourColor;\n"
		"	aColor = vec4(0,0,0,0);"
		"	for(int i=0;i<lightCount;i++){\n"
		"		vec3 totLight = lights[i].ambient+max(dot(normal,normalize(lights[i].pos-FragPos)),0)*lights[i].diffuse;\n"
		"		aColor += tempColor*(vec4(totLight,1.0));\n" //#TODO �������ֹ�����Ҫ��
		"	}\n"
		"   "
		"}\n\0";

	std::string fragmentShaderSource_LightedMesh =
		"#version 330 core\n"
		"out vec4 FragColor;\n"
		"in vec4 aColor;\n"
		"void main(){\n"
		"	FragColor = aColor;"
		"}\n";

	const char *geoShaderSource_Line =
		"#version 330																							   \n"
		"																										   \n"
		"layout(lines) in;                              // now we can access 2 vertices						   \n"
		"layout(triangle_strip, max_vertices = 4) out;  // always (for now) producing 2 triangles (so 4 vertices)  \n"
		"																										   \n"
		"uniform vec2  u_viewportSize;																			   \n"
		"uniform float u_thickness = 4;																			   \n"
		"																										   \n"
		"void main()																							   \n"
		"{																										   \n"
		"	vec4 p1 = gl_in[0].gl_Position;																		   \n"
		"	vec4 p2 = gl_in[1].gl_Position;																		   \n"
		"																										   \n"
		"	vec2 dir = normalize((p2.xy - p1.xy) * u_viewportSize);												   \n"
		"	vec2 offset = vec2(-dir.y, dir.x) * u_thickness / u_viewportSize;									   \n"
		"																										   \n"
		"	gl_Position = p1 + vec4(offset.xy * p1.w, 0.0, 0.0);												   \n"
		"	EmitVertex();																						   \n"
		"	gl_Position = p1 - vec4(offset.xy * p1.w, 0.0, 0.0);												   \n"
		"	EmitVertex();																						   \n"
		"	gl_Position = p2 + vec4(offset.xy * p2.w, 0.0, 0.0);												   \n"
		"	EmitVertex();																						   \n"
		"	gl_Position = p2 - vec4(offset.xy * p2.w, 0.0, 0.0);												   \n"
		"	EmitVertex();																						   \n"
		"																										   \n"
		"	EndPrimitive();																						   \n"
		"}																										   \n";

	PPolygon* PPolygon::currentTessPolygon = nullptr;

	PPolygon::~PPolygon()
	{
		for (int i = 0; i < vbos.size(); i++) {
			vbos[i]->destroy();
			vaos[i]->destroy();
		}
	}

	void PPolygon::initialize()
	{
		this->initializeOpenGLFunctions();

		if (!checkNormal(this->normal_)) {
			//TODO
			return;
		}

		GLUtesselator * tessobj;
		tessobj = gluNewTess();

		currentTessPolygon = this;

		gluTessProperty(tessobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_POSITIVE);
		gluTessNormal(tessobj, normal_.x, normal_.y, normal_.z);

		//ע��ص�����  
		gluTessCallback(tessobj, GLU_TESS_VERTEX, (void (CALLBACK *)())vertexCallback);
		gluTessCallback(tessobj, GLU_TESS_BEGIN, (void (CALLBACK *)())beginCallback);
		gluTessCallback(tessobj, GLU_TESS_END, (void (CALLBACK *)())endCallback);
		gluTessCallback(tessobj, GLU_TESS_ERROR, (void (CALLBACK *)())errorCallback);

		//gluTessCallback(tessobj, GLU_TESS_COMBINE, (void (CALLBACK *)())combineCallback);//����ͱ����ཻ������»ص��ûص����� 

		gluTessBeginPolygon(tessobj, NULL);
		int count = 0;

		for (const auto& pts : lps_)
		{
			for (const PType3f& pt : pts)
			{
				++count; //ͳ�����ж�����
			}
		}

		std::vector<GLdouble> tempCd(count*3);
		count = 0;
		for (const auto& pts : lps_)
		{
			gluTessBeginContour(tessobj);//���ö���εı��� 	
			for(const PType3f& pt : pts)
			{
				//����Ҫ��һ���ֲ������Ŀռ����洢����ֵ����������ʱ���飬Ҳ������һ����vector����push_back���棬��Ϊpushback�����е�ַ���
				tempCd[count * 3 + 0]=pt.x;
				tempCd[count * 3 + 1]=pt.y;
				tempCd[count * 3 + 2]=pt.z;
				gluTessVertex(tessobj, &tempCd[count*3], &tempCd[count*3]);
				++count;
			}
			gluTessEndContour(tessobj);
		}

		gluTessEndPolygon(tessobj);

		gluDeleteTess(tessobj);

		if (!compileVrtShader(shaderProgram, vertexShaderSource_Mesh, fragmentShaderSource_Mesh)) return;
		if (!compileVrtShader(lightShaderProgram, vertexShaderSource_LightedMesh.c_str(), fragmentShaderSource_LightedMesh.c_str())) return;
		if (!compileVrtShader(lineShaderProgram, vertexShaderSource_Mesh, geoShaderSource_Line,fragmentShaderSource_Mesh)) return;

		for (int i = 0; i < tessPts_.size(); i++) {
			vaos.emplace_back(new QOpenGLVertexArrayObject);
			vbos.emplace_back(new QOpenGLBuffer);
			auto& vao = vaos.back();
			auto& vbo = vbos.back();

			vao->create();
			vao->bind();
			vbo->create();
			vbo->bind();
			vbo->allocate(&tessPts_[i][0], tessPts_[i].size() * sizeof(Float));

			int attr = -1;
			attr = shaderProgram.attributeLocation("aPos");
			shaderProgram.setAttributeBuffer(attr, GL_FLOAT, 0, 3, 0);
			shaderProgram.enableAttributeArray(attr);

			attr = lightShaderProgram.attributeLocation("aPos");
			lightShaderProgram.setAttributeBuffer(attr, GL_FLOAT, 0, 3, 0);
			lightShaderProgram.enableAttributeArray(attr);
		}

		for (int i = 0; i < lps_.size(); i++) {
			boundPts_.emplace_back();
			for (int j = 0; j < lps_[i].size(); j++) {
				boundPts_.back().push_back(lps_[i][j].x);
				boundPts_.back().push_back(lps_[i][j].y);
				boundPts_.back().push_back(lps_[i][j].z);
			}
		}

		for (int i = 0; i < boundPts_.size(); i++) {
			linevaos.emplace_back(new QOpenGLVertexArrayObject);
			linevbos.emplace_back(new QOpenGLBuffer);
			auto& vao = linevaos.back();
			auto& vbo = linevbos.back();

			vao->create();
			vao->bind();
			vbo->create();
			vbo->bind();
			vbo->allocate(&boundPts_[i][0], boundPts_[i].size() * sizeof(Float));

			int attr = -1;
			attr = shaderProgram.attributeLocation("aPos");
			shaderProgram.setAttributeBuffer(attr, GL_FLOAT, 0, 3, 0);
			shaderProgram.enableAttributeArray(attr);
		}

		//Float* aaa = new Float[9]{ 0,1,0,1,1,0,0,0,0 };

		//linevaos.emplace_back(new QOpenGLVertexArrayObject);
		//linevbos.emplace_back(new QOpenGLBuffer);
		//auto& vao = linevaos.back();
		//auto& vbo = linevbos.back();

		//vao->create();
		//vao->bind();
		//vbo->create();
		//vbo->bind();
		//vbo->allocate(aaa, 9 * sizeof(Float));

		//int attr = -1;
		//attr = shaderProgram.attributeLocation("aPos");
		//shaderProgram.setAttributeBuffer(attr, GL_FLOAT, 0, 3, 0);
		//shaderProgram.enableAttributeArray(attr);
	}

	void PPolygon::paint(PaintInfomation* info)
	{
		if (info->fillmode == FILL || info->fillmode == FILL_WIREFRAME) {
			for (int i = 0; i < vaos.size(); i++) {
				vaos[i]->bind();
				if (info->lights.size())
				{
					lightShaderProgram.bind();
					lightShaderProgram.setUniformValue("modelMat", QMatrix4x4());
					lightShaderProgram.setUniformValue("viewMat", info->viewMat);
					lightShaderProgram.setUniformValue("projMat", info->projMat);
					lightShaderProgram.setUniformValue("ourColor", color().x, color().y, color().z, 1.0f);
					lightShaderProgram.setUniformValue("lightCount", GLint(info->lights.size()));
					//auto viewNormal = QVector3D((info->viewMat).inverted().transposed()*QVector4D(QVector3D(normal_), 0));
					//if (viewNormal.z() < 0) viewNormal = -viewNormal;
					auto viewNormal = QVector3D((info->viewMat).inverted().transposed()*QVector4D(QVector3D(normal_), 0));
					lightShaderProgram.setUniformValue("normal", viewNormal);
					for (int j = 0; j < info->lights.size(); j++) {
						std::string lightname = ("lights[" + std::to_string(j) + "]").c_str();
						lightShaderProgram.setUniformValue((lightname + ".ambient").c_str(), QVector3D(info->lights[j]->getAmbient()));
						lightShaderProgram.setUniformValue((lightname + ".pos").c_str(), QVector3D(info->lights[j]->getPosition()));
						lightShaderProgram.setUniformValue((lightname + ".diffuse").c_str(), QVector3D(info->lights[j]->getDiffuse()));
					}
				}
				else {
					shaderProgram.bind();
					shaderProgram.setUniformValue("modelMat", QMatrix4x4());
					shaderProgram.setUniformValue("viewMat", info->viewMat);
					shaderProgram.setUniformValue("projMat", info->projMat);

					shaderProgram.setUniformValue("ourColor", color().x, color().y, color().z, 1.0f);
				}
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

				glDrawArrays(drawTypes_[i], 0, tessPts_[i].size() / 3);

			}
		}
		if (info->fillmode == WIREFRAME || info->fillmode == FILL_WIREFRAME) {
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(-1, -1);
			for (int i = 0; i < linevaos.size(); i++) {
				// #PERF2 ����vao����ɫ�����Բ����ط���ô��
				linevaos[i]->bind();
				lineShaderProgram.bind();
				lineShaderProgram.setUniformValue("modelMat", QMatrix4x4());
				lineShaderProgram.setUniformValue("viewMat", info->viewMat);
				lineShaderProgram.setUniformValue("projMat", info->projMat);
				lineShaderProgram.setUniformValue("ourColor", .0, .0, .0, 1.0f);
				lineShaderProgram.setUniformValue("u_viewportSize", info->width, info->height);
				lineShaderProgram.setUniformValue("u_thickness", GLfloat(info->lineWidth));
				glDrawArrays(GL_LINE_LOOP, 0, boundPts_[i].size()/3);
			}
			glDisable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(0,0);
		}
		lineShaderProgram.release();
	}

	bool PPolygon::checkNormal(Vec3f& normal)
	{
		if (lps_.empty()) return false;
		const auto&  pts = lps_[0];
		if (pts.size() <= 2) return false;

		//�ж϶�����Ƿ���yzƽ����
		int n = pts.size();
		double result = 0;
		for (int i = 1; i < pts.size(); i++) {
			int aaa = (i - 1) % n;
			result += abs(Dot(Normalize(pts[i%n] - pts[(i - 1 + n) % n]), Vec3f(1, 0, 0)));
		}
		int axis;
		if (result > 0.5) axis = 0; //����yzƽ���ϣ�ѡ��x��
		else axis = 1; //����ѡ��y��

		int maxVal = -Infinity;
		int maxPt = -1;
		for (int i = 0; i < pts.size();i++) {
			if (pts[i][axis] > maxVal) {
				maxVal = pts[i][axis];
				maxPt = i;
			}
		}

		Vec3f trueNormal = Normalize(Cross(pts[maxPt] - pts[(maxPt - 1 + n) % n], pts[(maxPt + 1) % n] - pts[maxPt]));

		bool first = 1;
		result = 0;
		int cnt = 0;
		for (const auto& lp : lps_) {
			for (int i = 1; i < pts.size() - 1; i++) {
				Vec3f tempNormal = Cross(pts[i] - pts[(i - 1 + n) % n], pts[(i + 1) % n] - pts[i]); //
				if (tempNormal.Length() < 1e-14) continue;
				tempNormal.Normalize();
				result += abs(Dot(tempNormal, trueNormal));
				cnt++;
				if (first) {
					first = false;
				}
			}
		}

		if (first) return false;
		else if (abs(cnt - result)/cnt > 1e-6) {
			return false;
		}
		normal = trueNormal;

		return true;
	}

	vrt::Bounds3f PPolygon::getBound()
	{
		Bounds3f bd;
		for (const auto& lp : lps_)
		{
			for(const auto& pt : lp)
			{
				bd = Union(bd, Point3f(pt));
			}
		}
		return bd;
	}

	void CALLBACK vertexCallback(GLvoid* vertex)
	{
		double* cd = (double*)vertex;
		PPolygon::currentTessPolygon->tessPts_.back().push_back(cd[0]);
		PPolygon::currentTessPolygon->tessPts_.back().push_back(cd[1]);
		PPolygon::currentTessPolygon->tessPts_.back().push_back(cd[2]);
	}

	void CALLBACK beginCallback(GLenum type)
	{
		PPolygon::currentTessPolygon->drawTypes_.push_back(type);
		PPolygon::currentTessPolygon->tessPts_.push_back(std::vector<Float>());
	}

	void CALLBACK endCallback()
	{

	}

	void CALLBACK errorCallback(GLenum errorCode)
	{
		qDebug() << "error:" << errorCode;
	}
}
