///////////////////////////////////////////////////////////////////////////
//
//   MOOS - Mission Oriented Operating Suite 
//  
//   A suit of Applications and Libraries for Mobile Robotics Research 
//   Copyright (C) 2001-2005 Massachusetts Institute of Technology and 
//   Oxford University. 
//	
//   This software was written by Paul Newman at MIT 2001-2002 and Oxford 
//   University 2003-2005. email: pnewman@robots.ox.ac.uk. 
//	  
//   This file is part of a  MOOS Core Component. 
//		
//   This program is free software; you can redistribute it and/or 
//   modify it under the terms of the GNU General Public License as 
//   published by the Free Software Foundation; either version 2 of the 
//   License, or (at your option) any later version. 
//		  
//   This program is distributed in the hope that it will be useful, 
//   but WITHOUT ANY WARRANTY; without even the implied warranty of 
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
//   General Public License for more details. 
//			
//   You should have received a copy of the GNU General Public License 
//   along with this program; if not, write to the Free Software 
//   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 
//   02111-1307, USA. 
//
//////////////////////////    END_GPL    //////////////////////////////////

#include "MOOSSkewFilter.h"
#include <cmath>

namespace MOOS
{

	void CMOOSSkewFilter::DumpState()
	{
		m_env.DumpState();
	}


	void CConvexEnvelope::DumpState()
	{
		MOOSTrace("%d Segments\n", GetNumSegs());
		for (unsigned int i=0; i<m_segs.size(); i++)
		{
			m_segs[i].DumpState();
		}
	}

	CMOOSSkewFilter::CMOOSSkewFilter()
	{
		Reset();
	}


	void CMOOSSkewFilter::Reset()
	{
		m_dfLastVal    = 0.0;
		m_dfLastTime   = 0.0;
		m_nMeas        = 0;
		m_env.Reset();
	}
	

	double CMOOSSkewFilter::SmoothingFilter(double dfDT, double dfOldFilterVal, double dfNewMeas, double dfGradient) const
	{
		// UP filter is much faster than down filter
		const double alpha_up = 0.01;
		const double alpha_dn = 0.0001;
		
		double pred = dfOldFilterVal + dfGradient * dfDT;

		double innov = dfNewMeas - pred;

		if (innov > 0)
		{
			return pred + alpha_up * innov;
		}
		else
		{
			return pred + alpha_dn * innov;
		}
	}


	double CMOOSSkewFilter::Update(double dfTXtime, double dfRXtime, double dfTransportDelay, tSkewInfo *skewinfo)
	{
		double dfSkew = dfTXtime - dfRXtime;
		
		if (!m_env.AddPoint(dfTXtime, dfSkew))
		{
			// Probably has happened because we've got the same time
			// stamp as the previous measurement.  But could be because
			// something's gone more wrong with the convex envelope
			// Until we get better error info from AddPoint
			// we'll just have to assume the worst
			m_env.Reset();
		}

		// Get current best estimate of drift rate and offset		
		// But we'll only make a prediction if we've had a reasonable
		// number of samples in already.  Have to wait for the convex
		// envelope to settle down.
		bool bUseEstimate = (m_nMeas > 50);

		double dfGradient = 0.0;
		CConvexEnvelope::tSeg seg;
		if (bUseEstimate && m_env.GetLongestSeg(seg))
		{
			// Update skew using a prediction from the convex envelope
			dfSkew = seg.dfM * dfTXtime + seg.dfC;
			dfGradient = seg.dfM;

			if (skewinfo)
			{
				skewinfo->m       = dfGradient;
				skewinfo->c       = seg.dfC;
				skewinfo->envpred = dfSkew;
			}
		}

		// We'll push the result through a filter to ensure that 
		// skew values don't change too quickly
		if (m_nMeas > 0)
		{
			double dt = dfTXtime - m_dfLastTime;
			double dfFiltOut = SmoothingFilter(dt, m_dfLastVal, dfSkew, dfGradient);
			dfSkew = dfFiltOut;
		}

		if (m_env.GetNumSegs() > 500)
		{
			// It's highly unlikely ever to fill up this much but just in case...
			double dfMaxPeriod_hrs = 3.0;
			double dfMaxPeriod_s   = dfMaxPeriod_hrs * 60.0*60.0;
			m_env.CropFrontBefore(dfTXtime - dfMaxPeriod_s);
		}

		m_dfLastVal  = dfSkew;
		m_dfLastTime = dfTXtime;

		m_nMeas++;

		return dfSkew;
	}


	CConvexEnvelope::CConvexEnvelope()
	{
		Reset();
	}

	void CConvexEnvelope::Reset()
	{
		m_segs.clear();
		m_bHaveInitPt     =  false;
		m_InitPt          =  tPt(0,0);
		m_uiLongestSegID   = -1;
		m_dfLongestSegLen =  0;
	}


	bool CConvexEnvelope::GetLongestSeg(tSeg &seg) const
	{
		if (GetNumSegs() < 1) return false;
		if (m_uiLongestSegID >= GetNumSegs()) return false;
		seg = m_segs[m_uiLongestSegID];
		return true;
	}



	bool CConvexEnvelope::AddPoint(double x, double y)
	{
		tPt pt(x, y);

		if (GetNumSegs() == 0 && !m_bHaveInitPt)
		{
			// Store this initial point
			m_InitPt = pt;
			m_bHaveInitPt = true;
			return true;
		}

		// Add a new segment
		if (!m_bHaveInitPt)
			return false;

		tSeg seg;
		if (!MakeSeg(seg, m_InitPt, pt)) return false;
		AppendSeg(seg);		


		// Now try to merge segments from the back
		bool bMerged = true;
		while (GetNumSegs()>1 && bMerged)
		{
			bMerged = MergeLastSeg();
		}	

		// Store latest point for use next time
		// a point is added
		m_InitPt = pt;

		return true;

	}

	
	void CConvexEnvelope::AppendSeg(const tSeg & seg)
	{
		m_segs.push_back(seg);

		tSeg longestSeg;
		if (GetLongestSeg(longestSeg))
		{
			if (longestSeg.dfPeriod < seg.dfPeriod)
			{
				m_uiLongestSegID  = GetNumSegs()-1;
				m_dfLongestSegLen = seg.dfPeriod;
			}
		}
		else
		{
			m_uiLongestSegID  = 0;
			m_dfLongestSegLen = seg.dfPeriod;
		}
	}


	void CConvexEnvelope::CropFrontBefore(double x_min)
	{
		while (GetNumSegs() > 0 && m_uiLongestSegID > 0)
		{
			if (m_segs[0].p2.x < x_min)
			{
				m_segs.pop_front();
				m_uiLongestSegID -= 1;
			}
		}
	}


	bool CConvexEnvelope::MakeSeg(tSeg &seg, const tPt &p1, const tPt &p2) const
	{
		double M=0, C=0;

		if (!GetLineParams(p1, p2, M, C)) return false;
		
		tSeg s;
		s.p1 = p1;
		s.p2 = p2;
		s.dfM = M;
		s.dfC = C;
		s.dfPeriod = fabs(p2.x - p1.x);

		seg = s;
		return true;
	}

	bool CConvexEnvelope::GetLineParams(const tPt &p1, const tPt &p2, double &M, double &C) const
	{
		if (p1.x == p2.x) return false;

		M = (p2.y-p1.y) / (p2.x - p1.x);
		C = p2.y - M*p2.x;

		return true;
	}


	bool CConvexEnvelope::MergeLastSeg()
	{
		// Try to merge the latest segment with the one before

		if (GetNumSegs() < 2) return false;

		// Get merge candidates
		tSeg seg1 = *(m_segs.rbegin()+1);
		tSeg seg2 = *(m_segs.rbegin());

		if (seg2.dfM < seg1.dfM) return false;


		// Later segment is less steep, so a merge is possible

		if (m_uiLongestSegID >= GetNumSegs()-1)
		{
			// Ensure we check again for max segment length
			m_dfLongestSegLen = 0;
		}

		tSeg mergedSeg;
		if (!MakeSeg(mergedSeg, seg1.p1, seg2.p2))
		{
			return false;
		}

		mergedSeg.dfPeriod = seg1.dfPeriod + seg2.dfPeriod;

		m_segs.pop_back();
		m_segs.pop_back();
		m_segs.push_back(mergedSeg);

		if (mergedSeg.dfPeriod > m_dfLongestSegLen)
		{
			m_uiLongestSegID = GetNumSegs()-1;
			m_dfLongestSegLen = mergedSeg.dfPeriod;
		}

		return true;

	}

}


