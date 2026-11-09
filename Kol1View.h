// Kol1View.h : interface of the CKol1View class  (I kolokvijum 2023 - Robot)
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
	// 1. slike
	DImage* m_pGlava;
	DImage* m_pNadkolenica;
	DImage* m_pNadlaktica;
	DImage* m_pPodkolenica;
	DImage* m_pPodlaktica;
	DImage* m_pSaka;
	DImage* m_pStopalo;
	DImage* m_pTelo;
	DImage* m_pPozadina;

	// 6. atributi (uglovi u stepenima)
	double m_angleNadlaktica;   // G / H  [-10, 90]
	double m_anglePodlaktica;   // D / F  [-10, 80]
	double m_angleSaka;         // A / S  [-10, 30]
	double m_angleRobot;        // 1 / 2  rotacija celog robota
	double m_scaleRobot;        // 1 / 2  skaliranje celog robota
	double m_moveX;             // <- / -> pomeranje celog robota levo-desno

	// 3., 4.
	void DrawBackground(CDC* pDC);
	void DrawImgTransparent(CDC* pDC, DImage* pImage);

	// 5.
	void Translate(CDC* pDC, float dX, float dY, bool rightMultiply);
	void Rotate(CDC* pDC, float angle, bool rightMultiply);   // angle u stepenima
	void Scale(CDC* pDC, float sX, float sY, bool rightMultiply);

	// 7. - 9.
	void DrawHalf(CDC* pDC);
	void DrawHead(CDC* pDC);
	void DrawRobot(CDC* pDC);

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
