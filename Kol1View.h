// Kol1View.h : interface of the CKol1View class  (I kolokvijum 2018 - Bager)
#pragma once
#include "DImage.h"

class CKol1View : public CView
{
protected:
	CKol1View() noexcept;
	DECLARE_DYNCREATE(CKol1View)

public:
	CKol1Doc* GetDocument() const;

	virtual void OnDraw(CDC* pDC);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual ~CKol1View();

protected:
	// 1. slike i metafajl
	DImage* m_pBager;
	DImage* m_pArm1;
	DImage* m_pArm2;
	DImage* m_pPozadina;
	HENHMETAFILE m_hFork;     // viljuska.emf
	CRect m_rcFork;           // okvirni pravougaonik metafajla

	// 10. atributi (uglovi u stepenima, pomeraj u pikselima)
	double m_angleArm1;       // 1 / 2
	double m_angleArm2;       // 3 / 4
	double m_angleFork;       // 5 / 6
	double m_moveX;           // <- / ->

	// 2.
	void Translate(CDC* pDC, float dX, float dY, bool rightMultiply);
	void Rotate(CDC* pDC, float angle, bool rightMultiply);   // angle u stepenima
	void Scale(CDC* pDC, float sX, float sY, bool rightMultiply);

	// 3., 4.
	void DrawBackground(CDC* pDC);
	void DrawImgTransparent(CDC* pDC, DImage* pImage);

	// 5. - 9.
	void DrawBody(CDC* pDC);
	void DrawArm1(CDC* pDC);
	void DrawArm2(CDC* pDC);
	void DrawFork(CDC* pDC);
	void DrawExcavator(CDC* pDC);

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};

#ifndef _DEBUG
inline CKol1Doc* CKol1View::GetDocument() const
   { return reinterpret_cast<CKol1Doc*>(m_pDocument); }
#endif
