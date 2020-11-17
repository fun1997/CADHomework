#include <QtWidgets/QApplication>
#include "Scene.h"
#include "PPolygon.h"
#include <vector>
#include "cad.h"
using namespace vrt;

/*Vertex* newv(){
	double a,b,c;
	cin>>a>>b>>c;
	Vertex* v=new Vertex(a,b,c);
	return v;
}*/
//�޴������������һ��ʵ�塢���桢�»�������ʵ������ָ��
Solid* mvsf() {
	Solid* ob = new Solid;
	Face* f = new Face;
	Loop* lp = new Loop;
	ob->sfaces = f;
	FACE.push_back(f);
	f->fsolid = ob;
	f->floops = lp;
	f->finloops = nullptr;
	lp->lface = f;
	lp->ledg = nullptr;
	return ob;
}
//�������������ֱ�Ϊһ���е㡢�µ㡢�����е����ڵĻ�������һ���±ߣ����ػ��еİ��ָ��
Halfedge* mev(Vertex* v1, Vertex* v, Loop*lp) {
	Halfedge* he1;
	Halfedge* he2;
	Halfedge* e;
	Edge* edg;
	he1 = new Halfedge;
	he2 = new Halfedge;
	edg = new Edge;
	he1->startv = v1;
	he1->endv = v;
	he2->startv = v;
	he2->endv = v1;
	he1->bro = he2;
	he2->bro = he1;
	edg->he1 = he1;
	edg->he2 = he2;
	he1->edg = edg;
	he2->edg = edg;
	he1->wloop = lp;
	he2->wloop = lp;
	he1->nxt = he2;
	he2->prv = he1;
	if (lp->ledg == NULL) {
		he2->nxt = he1;
		he1->prv = he2;
		lp->ledg = he1;
	}
	else {
		for (e = lp->ledg; e->startv != v1; e = e->nxt);
		he2->nxt = e;
		e->prv->nxt = he1;
		he1->prv = e->prv;
		e->prv = he2;
	}
	return he1;
}
//����ͬһ�⻷�е������㣬����һ���±ߣ�ͬʱ����һ���»�������,������v2Ϊ�����ʼ�����ָ��
Face* mef(Vertex* v1, Vertex* v2, Loop* lp1) {
	Edge* edg = new Edge;
	Face* f = new Face;
	Loop* lp2 = new Loop;
	Halfedge* he1 = new Halfedge;
	Halfedge* he2 = new Halfedge;
	Halfedge* e1 = new Halfedge;
	Halfedge* e2 = new Halfedge;
	FACE.push_back(f);
	he1->startv = v1;
	he1->endv = v2;
	he2->startv = v2;
	he2->endv = v1;
	he1->bro = he2;
	he2->bro = he1;
	he1->edg = edg;
	he2->edg = edg;
	edg->he1 = he1;
	edg->he2 = he2;
	for (e1 = lp1->ledg; e1->startv != v1; e1 = e1->nxt);
	for (e2 = lp1->ledg; e2->startv != v2; e2 = e2->nxt);
	he2->nxt = e1;
	e1->prv->nxt = he1;
	he1->prv = e1->prv;
	e1->prv = he2;
	e2->prv->nxt = he2;
	he2->prv = e2->prv;
	he1->nxt = e2;
	e2->prv = he1;
	lp1->ledg = he1;
	lp2->ledg = he2;
	lp2->lface = f;
	f->floops = lp2;
	return f;
}
//��ȥ���е�һ���ߣ���ͬһ���⻷�����һ���ڻ����˴�����Ҫɾ���ߵ���ʼ�����ֹ�㣬��ʼ�����⻷�ϣ���ֹ�����ڻ��ϣ������ڻ����ڵ���ָ��
Face* kemr(Vertex *v1,Vertex *v2, Loop* lp) {
	Halfedge* e1 = new Halfedge;
	Halfedge* e2 = new Halfedge;
	Loop* inlp = new Loop;
	e1 = lp->ledg;
	e2 = lp->ledg;
	while (e1->startv != v1 || e1->endv != v2) {
		e1 = e1->nxt;
	}
	while (e2->startv != v2 || e2->endv != v1) {
		e2 = e2->nxt;
	}
	e1->nxt->prv = e2->prv;
	e2->prv->nxt = e1->nxt;
	e1->prv->nxt = e2->nxt;
	e2->nxt->prv = e1->prv;
	inlp->ledg = e1->nxt;
	inlp->lface = lp->lface;
	if (lp->lface->finloops == nullptr) {
		lp->lface->finloops = inlp;
	}
	else {
		Loop* tmplp;
		for (tmplp = lp->lface->finloops; tmplp->nextl != nullptr; tmplp = tmplp->nextl);
		tmplp->nextl = inlp;
	}
	return inlp->lface;

}
//ɾ��һ����f2�������䶨��Ϊһ���ڻ�����������������һ������������ϳ�һ�����壬�����ڻ����ڵ���ָ��
Face* kfmrh(Face* f1, Face* f2) {
	if (f1->finloops == nullptr) {
		f1->finloops = f2->floops;
	}
	else {
		Loop* lp;
		for (lp = f1->finloops; lp->nextl != nullptr; lp = lp->nextl);
		lp->nextl = f2->floops;
	}
	std::vector<Face *>::iterator ite;
	for (ite = FACE.begin(); ite != FACE.end();) {
		if (*ite == f2) {
			ite = FACE.erase(ite);
		}
		else
		{
			++ite;
		}
	}
	delete  f2;
	return f1;
}
//����Ҫɨ�ɵ��桢�����Լ����ȣ�����ɨ�ɲ���������ɨ�ɺ����
Face* sweep(Face* f, Vec* vec, int L) {
	Loop* lp = new Loop;
	Face* tmpf = new Face;
	Halfedge* tmp = new Halfedge;
	Halfedge* phe = new Halfedge;
	Vertex *firstv, *firstup, *prevup, *nextv;
	lp = f->floops;
	phe = lp->ledg;
	firstv = phe->startv;
	UP.push_back(new Vertex());
	firstup = new Vertex;
	firstup->x = firstv->x + vec->x*L;
	firstup->y = firstv->y + vec->y*L;
	firstup->z = firstv->z + vec->z*L;
	UP.back()=firstup;
	tmp = mev(firstv, firstup, lp);
	prevup = firstup;
	phe = phe->nxt;
	nextv = phe->startv;
	tmpf = f;
	while (nextv != firstv) {
		UP.push_back(new Vertex());
  		UP.back()->x = nextv->x + vec->x*L;
		UP.back()->y = nextv->y + vec->y*L;
		UP.back()->z = nextv->z + vec->z*L;
		mev(nextv, UP.back(), tmpf->floops);
		tmpf = mef(UP.back(), prevup, tmpf->floops);
		prevup = UP.back();
		phe = phe->nxt;
		nextv = phe->startv;
	}
	tmpf = mef(firstup, prevup, tmpf->floops);
	return tmpf;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
//�½�����
	Scene scene;
	int num,i;
	Halfedge *pe=new Halfedge;
	Halfedge *ne = new Halfedge;
	Vertex *fv = new Vertex;
	Vertex *nv = new Vertex;
	Solid *obj = new Solid;
	Halfedge *tmpe, *tmp;
	Face *tmpf = new Face;
	Face *tmpf2 = new Face;
	Loop *tmplp = new Loop;
	num = 0;
//�½��㼯
	std::vector<Vertex*> vertex = { new Vertex(0,0,0),new Vertex(0,1,0),new Vertex(1,1,0),new Vertex(1,0,0),
						   new Vertex(0,0,1),new Vertex(0,1,1),new Vertex(1,1,1),new Vertex(1,0,1),
						   new Vertex(0.1,0.1,1),new Vertex(0.1,0.4,1),new Vertex(0.4,0.4,1),new Vertex(0.5,0.3,1),
						   new Vertex(0.1,0.1,0),new Vertex(0.1,0.4,0),new Vertex(0.4,0.4,0),new Vertex(0.4,0.1,0),
						   new Vertex(0.6,0.2,0),new Vertex(0.6,0.9,0),new Vertex(0.9,0.9,0),new Vertex(0.9,0.2,0) };
//�½�����
	std::vector<Vec*> v = { new Vec(0,0.5,0.5),new Vec(0,0.4,0.5),new Vec(0,0.3,0.5),new Vec(0,0.2,0.5),new Vec(0,0.1,0.5) };
	obj = mvsf();
//�½�����
	Face *outf= new Face;
	mev(vertex[0], vertex[1], obj->sfaces->floops);
	mev(vertex[1], vertex[2], obj->sfaces->floops);
	mev(vertex[2], vertex[3], obj->sfaces->floops);
	outf=mef(vertex[3], vertex[0], obj->sfaces->floops);
//ɨ���㷨�����ж�ʵ��
//�ڵ����½��ڻ�
	Face *inlpf1= new Face;
	mev(vertex[0], vertex[12], obj->sfaces->floops);
	mev(vertex[12], vertex[13], obj->sfaces->floops);
	mev(vertex[13], vertex[14], obj->sfaces->floops);
	mev(vertex[14], vertex[15], obj->sfaces->floops);
	kemr(vertex[0], vertex[12], obj->sfaces->floops);
	inlpf1= mef(vertex[12], vertex[15], obj->sfaces->finloops);
//�½��ڻ�2
	Face *inlpf2 = new Face;
	mev(vertex[1], vertex[16], obj->sfaces->floops);
	mev(vertex[16], vertex[17], obj->sfaces->floops);
	mev(vertex[17], vertex[18], obj->sfaces->floops);
	mev(vertex[18], vertex[19], obj->sfaces->floops);
	kemr(vertex[1], vertex[16], obj->sfaces->floops);
	inlpf2=mef(vertex[16], vertex[19], obj->sfaces->finloops->nextl);
//�ֱ�����⻷��ɨ��
	outf=sweep(outf, v[0], 1);
	inlpf1=sweep(inlpf1, v[0], 1);
	outf = kfmrh(outf, inlpf1);
	inlpf2 = sweep(inlpf2, v[0], 1);
	outf = kfmrh(outf, inlpf2);	
/*
//ֱ��ȫ����ŷ����������ʵ��
	tmpe = mev(vertex[0], vertex[4], tmpf->floops);
	tmpe = mev(vertex[1], vertex[5], tmpf->floops);
	tmpe = mev(vertex[2], vertex[6], tmpf->floops);
	tmpe = mev(vertex[3], vertex[7], tmpf->floops);
	tmpf = mef(vertex[4], vertex[5], tmpf->floops);
	tmpf = mef(vertex[5], vertex[6], tmpf->floops);
	tmpf = mef(vertex[6], vertex[7], tmpf->floops);
	tmpf = mef(vertex[7], vertex[4], tmpf->floops);

	tmp = mev(vertex[4], vertex[8], tmpf->floops);
	tmpe = mev(vertex[8], vertex[9], tmpf->floops);
	tmpe = mev(vertex[9], vertex[10], tmpf->floops);
	tmpe = mev(vertex[10], vertex[11], tmpf->floops);
	tmpf = kemr(vertex[4],vertex[8], tmpf->floops);

	tmpf = mef(vertex[11], vertex[8], tmpf->finloops);


	tmpe = mev(vertex[8], vertex[12], tmpf->floops);
	tmpe = mev(vertex[9], vertex[13], tmpf->floops);
	tmpe = mev(vertex[10], vertex[14], tmpf->floops);
	tmpe = mev(vertex[11], vertex[15], tmpf->floops);
	tmpf = mef(vertex[12], vertex[13], tmpf->floops);
	tmpf = mef(vertex[13], vertex[14], tmpf->floops);
	tmpf = mef(vertex[14], vertex[15], tmpf->floops);
	tmpf = mef(vertex[15], vertex[12], tmpf->floops);
	tmpf = kfmrh(obj->sfaces, tmpf);
*/
//��չ-----���ٽ��ж��ɨ��
/*	for (i = 1; i < 4; i++) {
		FACE.push_back(new Face);
		FACE.push_back(new Face);
		FACE.push_back(new Face);
		int num1 = FACE.size() - 3;
		int num2 = FACE.size() - 2;
		int num3 = FACE.size() - 1;
		FACE[num1]->floops = outf->floops;
		FACE[num2]->floops = outf->finloops;
		FACE[num3]->floops = outf->finloops->nextl;
		outf = sweep(FACE[num1], v[i%5], 1);
		inlpf1 = sweep(FACE[num2], v[i%5], 1);
		outf = kfmrh(outf, inlpf1);
		inlpf2= sweep(FACE[num3], v[i%5], 1);
		outf = kfmrh(outf, inlpf2);
	}*/
	num = FACE.size();
//�����е����½�����Σ�������ͼ����ʾ
	std::vector<std::shared_ptr<PPolygon>> plg(num);
	std::vector<std::vector<std::vector<Point3f>>> lps(num );
	for (i = 0; i < num; i++) {
		pe = FACE[i]->floops->ledg;
		lps[i].push_back(std::vector<Point3f>());
		fv = pe->startv;
		lps[i].back().push_back(Point3f(fv->x,fv->y,fv->z));
		pe = pe->nxt;
		nv = pe->startv;
		while (nv != fv) {
			lps[i].back().push_back(Point3f(nv->x,nv->y,nv->z));
			pe = pe->nxt;
			nv = pe->startv;
		}
		tmplp = FACE[i]->finloops;
		while (tmplp != nullptr) {
			pe = tmplp->ledg;
			lps[i].push_back(std::vector<Point3f>());
			fv = pe->startv;
			lps[i].back().push_back(Point3f(fv->x, fv->y, fv->z));
			pe = pe->nxt;
			nv = pe->startv;
			while (nv != fv) {
				lps[i].back().push_back(Point3f(nv->x, nv->y, nv->z));
				pe = pe->nxt;
				nv = pe->startv;
			}
			tmplp = tmplp->nextl;
		}
		plg[i].reset(new PPolygon(lps[i]));
		scene.addPrimitive(plg[i]);
	}
	scene.show();
	scene.resize(1280, 720);

    return a.exec();
}
