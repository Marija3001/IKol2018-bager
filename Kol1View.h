
// Kol2018View.h : interface of the CKol2018View class
//

#pragma once
#include "../Kol1-2024/DImage.h"


class CKol2018View : public CView
{
protected: // create from serialization only
	CKol2018View() noexcept;
	DECLARE_DYNCREATE(CKol2018View)

// Attributes
public:
	CKol2018Doc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	void Translate(CDC* pDC, double dX, double dY, bool rightMultiply);
	void Rotate(CDC* pDC, float angle, bool rightMultiply);
	void Scale(CDC* pDC, float scaleX, float scaleY, bool rightMultiply);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	void DrawBackground(CDC* pDC);
	void DrawImgTransparent(CDC* pDC, DImage* pImage);
	void DrawBody(CDC* pDC);
	void DrawArm1(CDC* pDC);
	void DrawArm2(CDC* pDC);
	void DrawFork(CDC* pDC);
	void DrawExcavator(CDC* pDC);
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

	DImage* m_pBager;
	DImage* m_pArm1;
	DImage* m_pArm2;
	DImage* m_pPozadina;
	//metafajl viljuska
	HENHMETAFILE m_hViljuska;
	CRect m_rectViljuska; //okvirni pravougaonik za metafajl

	double m_angleArm1; //1-2
	double m_angleArm2;
	double m_angleFork;
	double m_moveX;


// Implementation
public:
	virtual ~CKol2018View();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};

#ifndef _DEBUG  // debug version in Kol2018View.cpp
inline CKol2018Doc* CKol2018View::GetDocument() const
   { return reinterpret_cast<CKol2018Doc*>(m_pDocument); }
#endif

