#pragma once

#include "MaxPayneKFExporter.h"

#define MPKFExporter_CLASS_ID       Class_ID(0x7f7d370d, 0x6e0c0771)

class MPKFExporter: public SceneExport
{
public:
    MPKFExporter();

    virtual ~MPKFExporter();

	int	ExtCount() override { return 1; }

    const MCHAR* Ext(int n) override { return _T("KF2"); }

	const MCHAR* LongDesc() override { return _T("Max Payne KF2 File"); }

	const MCHAR* ShortDesc() override { return _T("Max Payne KF2 File"); }
	
	const MCHAR* AuthorName() override { return _T("Max Payne KF2 File"); }

	const MCHAR* CopyrightMessage() override { return _T("Bolotaev Sergey Borisovich"); }

	const MCHAR* OtherMessage1() override { return _T("Max Payne KF2 File"); }

	const MCHAR* OtherMessage2() override { return _T(""); }

	unsigned int Version() override { return 100; }

	void ShowAbout(HWND hWnd) override {}

	/*
	#define IMPEXP_FAIL 0
	#define IMPEXP_SUCCESS 1
	#define IMPEXP_CANCEL 2
	 */
	virtual int	DoExport(const MCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0) override;
};