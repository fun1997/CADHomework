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
//无传入参数，生成一新实体、新面、新环，返回实体对象的指针
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
//传入三个参数分别为一已有点、新点、与已有点所在的环，生成一条新边，返回环中的半边指针
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
//给定同一外环中的两个点，构造一条新边，同时生成一个新环、新面,返还以v2为半边起始点的面指针
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
//消去环中的一条边，在同一个外环里，构造一个内环，此处输入要删除边的起始点和终止点，起始点在外环上，终止点在内环上，返回内环所在的面指针
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
//删除一个面f2，并将其定义为一个内环，进而在体中生成一个柄或将两物体合成一个物体，返回内环所在的面指针
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
//给定要扫成的面、方向以及长度，进行扫成操作，返回扫成后的面
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
    
//新建场景
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
//新建点集
	std::vector<Vertex*> vertex = { new Vertex(0,0,0),new Vertex(0,1,0),new Vertex(1,1,0),new Vertex(1,0,0),
						   new Vertex(0,0,1),new Vertex(0,1,1),new Vertex(1,1,1),new Vertex(1,0,1),
						   new Vertex(0.1,0.1,1),new Vertex(0.1,0.4,1),new Vertex(0.4,0.4,1),new Vertex(0.5,0.3,1),
						   new Vertex(0.1,0.1,0),new Vertex(0.1,0.4,0),new Vertex(0.4,0.4,0),new Vertex(0.4,0.1,0),
						   new Vertex(0.6,0.2,0),new Vertex(0.6,0.9,0),new Vertex(0.9,0.9,0),new Vertex(0.9,0.2,0) };
//新建方向集
	std::vector<Vec*> v = { new Vec(0,0.5,0.5),new Vec(0,0.4,0.5),new Vec(0,0.3,0.5),new Vec(0,0.2,0.5),new Vec(0,0.1,0.5) };
	obj = mvsf();
//新建底面
	Face *outf= new Face;
	mev(vertex[0], vertex[1], obj->sfaces->floops);
	mev(vertex[1], vertex[2], obj->sfaces->floops);
	mev(vertex[2], vertex[3], obj->sfaces->floops);
	outf=mef(vertex[3], vertex[0], obj->sfaces->floops);
//扫成算法生成有洞实体
//在底面新建内环
	Face *inlpf1= new Face;
	mev(vertex[0], vertex[12], obj->sfaces->floops);
	mev(vertex[12], vertex[13], obj->sfaces->floops);
	mev(vertex[13], vertex[14], obj->sfaces->floops);
	mev(vertex[14], vertex[15], obj->sfaces->floops);
	kemr(vertex[0], vertex[12], obj->sfaces->floops);
	inlpf1= mef(vertex[12], vertex[15], obj->sfaces->finloops);
//新建内环2
	Face *inlpf2 = new Face;
	mev(vertex[1], vertex[16], obj->sfaces->floops);
	mev(vertex[16], vertex[17], obj->sfaces->floops);
	mev(vertex[17], vertex[18], obj->sfaces->floops);
	mev(vertex[18], vertex[19], obj->sfaces->floops);
	kemr(vertex[1], vertex[16], obj->sfaces->floops);
	inlpf2=mef(vertex[16], vertex[19], obj->sfaces->finloops->nextl);
//分别对内外环作扫成
	outf=sweep(outf, v[0], 1);
	inlpf1=sweep(inlpf1, v[0], 1);
	outf = kfmrh(outf, inlpf1);
	inlpf2 = sweep(inlpf2, v[0], 1);
	outf = kfmrh(outf, inlpf2);	
/*
//直接全部用欧拉操作生成实体
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
//扩展-----可再进行多次扫成
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
//对所有的面新建多边形，并进行图形显示
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
