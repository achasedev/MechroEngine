///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: Nov 14th, 2021
/// Description: See below
///--------------------------------------------------------------------------------------------------------------------------------------------------

/*******************************************************
*                                                      *
*  volInt.c                                            *
*                                                      *
*  This code computes volume integrals needed for      *
*  determining mass properties of polyhedral bodies.   *
*                                                      *
*  For more information, see the accompanying README   *
*  file, and the paper                                 *
*                                                      *
*  Brian Mirtich, "Fast and Accurate Computation of    *
*  Polyhedral Mass Properties," journal of graphics    *
*  tools, volume 1, number 1, 1996.                    *
*                                                      *
*  This source code is public domain, and may be used  *
*  in any way, shape or form, free of charge.          *
*                                                      *
*  Copyright 1995 by Brian Mirtich                     *
*                                                      *
*  mirtich@cs.berkeley.edu                             *
*  http://www.cs.berkeley.edu/~mirtich                 *
	*                                                  *
*******************************************************/

/*
	Revision history

	26 Jan 1996	Program creation.

		3 Aug 1996	Corrected bug arising when polyhedron density
			is not 1.0.  Changes confined to function main().
			Thanks to Zoran Popovic for catching this one.

	27 May 1997     Corrected sign error in translation of inertia
					product terms to center of mass frame.  Changes
			confined to function main().  Thanks to
			Chris Hecker.
*/

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include "Engine/Physics/Rigidbody/VolumeIntegration.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#define MAX_VERTS 100     /* maximum number of polyhedral vertices */
#define MAX_FACES 100     /* maximum number of polyhedral faces */
#define MAX_POLYGON_SZ 10 /* maximum number of verts per polygonal face */

#define X 0
#define Y 1
#define Z 2

/*
	============================================================================
	macros
	============================================================================
*/

#define SQR(x) ((x)*(x))
#define CUBE(x) ((x)*(x)*(x))

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
typedef struct {
	int numVerts;
	double norm[3];
	double w;
	int verts[MAX_POLYGON_SZ];
	struct polyhedron *poly;
} FACE;

typedef struct polyhedron {
	int numVerts, numFaces;
	double verts[MAX_VERTS][3];
	FACE faces[MAX_FACES];
} POLYHEDRON;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------
static int A;   /* alpha */
static int B;   /* beta */
static int C;   /* gamma */

/* projection integrals */
static double P1, Pa, Pb, Paa, Pab, Pbb, Paaa, Paab, Pabb, Pbbb;

/* face integrals */
static double Fa, Fb, Fc, Faa, Fbb, Fcc, Faaa, Fbbb, Fccc, Faab, Fbbc, Fcca;

/* volume integrals */
static double T0, T1[3], T2[3], TP[3];

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
static void ComputeProjectionIntegrals(FACE *f)
{
	double a0, a1, da;
	double b0, b1, db;
	double a0_2, a0_3, a0_4, b0_2, b0_3, b0_4;
	double a1_2, a1_3, b1_2, b1_3;
	double C1, Ca, Caa, Caaa, Cb, Cbb, Cbbb;
	double Cab, Kab, Caab, Kaab, Cabb, Kabb;
	int i;

	P1 = Pa = Pb = Paa = Pab = Pbb = Paaa = Paab = Pabb = Pbbb = 0.0;

	for (i = 0; i < f->numVerts; i++) {
		a0 = f->poly->verts[f->verts[i]][A];
		b0 = f->poly->verts[f->verts[i]][B];
		a1 = f->poly->verts[f->verts[(i + 1) % f->numVerts]][A];
		b1 = f->poly->verts[f->verts[(i + 1) % f->numVerts]][B];
		da = a1 - a0;
		db = b1 - b0;
		a0_2 = a0 * a0; a0_3 = a0_2 * a0; a0_4 = a0_3 * a0;
		b0_2 = b0 * b0; b0_3 = b0_2 * b0; b0_4 = b0_3 * b0;
		a1_2 = a1 * a1; a1_3 = a1_2 * a1;
		b1_2 = b1 * b1; b1_3 = b1_2 * b1;

		C1 = a1 + a0;
		Ca = a1 * C1 + a0_2; Caa = a1 * Ca + a0_3; Caaa = a1 * Caa + a0_4;
		Cb = b1 * (b1 + b0) + b0_2; Cbb = b1 * Cb + b0_3; Cbbb = b1 * Cbb + b0_4;
		Cab = 3 * a1_2 + 2 * a1*a0 + a0_2; Kab = a1_2 + 2 * a1*a0 + 3 * a0_2;
		Caab = a0 * Cab + 4 * a1_3; Kaab = a1 * Kab + 4 * a0_3;
		Cabb = 4 * b1_3 + 3 * b1_2*b0 + 2 * b1*b0_2 + b0_3;
		Kabb = b1_3 + 2 * b1_2*b0 + 3 * b1*b0_2 + 4 * b0_3;

		P1 += db * C1;
		Pa += db * Ca;
		Paa += db * Caa;
		Paaa += db * Caaa;
		Pb += da * Cb;
		Pbb += da * Cbb;
		Pbbb += da * Cbbb;
		Pab += db * (b1*Cab + b0 * Kab);
		Paab += db * (b1*Caab + b0 * Kaab);
		Pabb += da * (a1*Cabb + a0 * Kabb);
	}

	P1 /= 2.0;
	Pa /= 6.0;
	Paa /= 12.0;
	Paaa /= 20.0;
	Pb /= -6.0;
	Pbb /= -12.0;
	Pbbb /= -20.0;
	Pab /= 24.0;
	Paab /= 60.0;
	Pabb /= -60.0;
}


//-------------------------------------------------------------------------------------------------
static void ComputeFaceIntegrals(FACE *f)
{
	double *n, w;
	double k1, k2, k3, k4;

	ComputeProjectionIntegrals(f);

	w = f->w;
	n = f->norm;
	k1 = 1 / n[C]; k2 = k1 * k1; k3 = k2 * k1; k4 = k3 * k1;

	Fa = k1 * Pa;
	Fb = k1 * Pb;
	Fc = -k2 * (n[A] * Pa + n[B] * Pb + w * P1);

	Faa = k1 * Paa;
	Fbb = k1 * Pbb;
	Fcc = k3 * (SQR(n[A])*Paa + 2 * n[A] * n[B] * Pab + SQR(n[B])*Pbb
		+ w * (2 * (n[A] * Pa + n[B] * Pb) + w * P1));

	Faaa = k1 * Paaa;
	Fbbb = k1 * Pbbb;
	Fccc = -k4 * (CUBE(n[A])*Paaa + 3 * SQR(n[A])*n[B] * Paab
		+ 3 * n[A] * SQR(n[B])*Pabb + CUBE(n[B])*Pbbb
		+ 3 * w*(SQR(n[A])*Paa + 2 * n[A] * n[B] * Pab + SQR(n[B])*Pbb)
		+ w * w*(3 * (n[A] * Pa + n[B] * Pb) + w * P1));

	Faab = k1 * Paab;
	Fbbc = -k2 * (n[A] * Pabb + n[B] * Pbbb + w * Pbb);
	Fcca = k3 * (SQR(n[A])*Paaa + 2 * n[A] * n[B] * Paab + SQR(n[B])*Pabb
		+ w * (2 * (n[A] * Paa + n[B] * Pab) + w * Pa));
}


//-------------------------------------------------------------------------------------------------
static void ComputeVolumeIntegrals(POLYHEDRON *p)
{
	FACE *f;
	double nx, ny, nz;
	int i;

	T0 = T1[X] = T1[Y] = T1[Z]
		= T2[X] = T2[Y] = T2[Z]
		= TP[X] = TP[Y] = TP[Z] = 0;

	for (i = 0; i < p->numFaces; i++) {

		f = &p->faces[i];

		nx = fabs(f->norm[X]);
		ny = fabs(f->norm[Y]);
		nz = fabs(f->norm[Z]);
		if (nx > ny && nx > nz) C = X;
		else C = (ny > nz) ? Y : Z;
		A = (C + 1) % 3;
		B = (A + 1) % 3;

		ComputeFaceIntegrals(f);

		T0 += f->norm[X] * ((A == X) ? Fa : ((B == X) ? Fb : Fc));

		T1[A] += f->norm[A] * Faa;
		T1[B] += f->norm[B] * Fbb;
		T1[C] += f->norm[C] * Fcc;
		T2[A] += f->norm[A] * Faaa;
		T2[B] += f->norm[B] * Fbbb;
		T2[C] += f->norm[C] * Fccc;
		TP[A] += f->norm[A] * Faab;
		TP[B] += f->norm[B] * Fbbc;
		TP[C] += f->norm[C] * Fcca;
	}

	T1[X] /= 2; T1[Y] /= 2; T1[Z] /= 2;
	T2[X] /= 3; T2[Y] /= 3; T2[Z] /= 3;
	TP[X] /= 2; TP[Y] /= 2; TP[Z] /= 2;
}


//-------------------------------------------------------------------------------------------------
static void ConvertEnginePolyToThirdPartyPoly(const Polyhedron& enginePoly, POLYHEDRON& tpPoly)
{
	tpPoly.numFaces = enginePoly.GetNumFaces();
	tpPoly.numVerts = enginePoly.GetNumVertices();

	ASSERT_OR_DIE(tpPoly.numFaces <= MAX_FACES, "Ran out of room for faces!");
	ASSERT_OR_DIE(tpPoly.numVerts <= MAX_VERTS, "Ran out of room for faces!");

	// Vertices
	for (int i = 0; i < tpPoly.numVerts; ++i)
	{
		Vector3 v = enginePoly.GetVertexPosition(i);

		for (int j = 0; j < 3; ++j)
		{
			tpPoly.verts[i][j] = static_cast<double>(v.data[j]);
		}
	}

	// Faces
	for (int i = 0; i < tpPoly.numFaces; ++i)
	{
		FACE* tpFace = &tpPoly.faces[i];
		const PolyhedronFace* engineFace = enginePoly.GetFace(i);

		tpFace->numVerts = static_cast<int>(engineFace->m_indices.size());
		for (int j = 0; j < tpFace->numVerts; ++j)
		{
			tpFace->verts[j] = engineFace->m_indices[j];
		}

		Plane3 facePlane = enginePoly.GetFaceSupportPlane(i);
		tpFace->w = -1.0f * facePlane.GetDistance();
		Vector3 normal = facePlane.GetNormal();
		for (int j = 0; j < 3; ++j)
		{
			tpFace->norm[j] = static_cast<double>(normal.data[j]);
		}

		tpFace->poly = &tpPoly;
	}
}


//-------------------------------------------------------------------------------------------------
Vector3 ComputeCenterOfMassAndInteriaTensor(const Polyhedron& poly, Matrix3& out_tensor, double mass)
{
	POLYHEDRON p;
	ConvertEnginePolyToThirdPartyPoly(poly, p);

	double density;
	double r[3];            /* center of mass */
	double J[3][3];         /* inertia tensor */

	ComputeVolumeIntegrals(&p);

	density = mass / T0;

	/* compute center of mass */
	r[X] = T1[X] / T0;
	r[Y] = T1[Y] / T0;
	r[Z] = T1[Z] / T0;

	/* compute inertia tensor */
	J[X][X] = density * (T2[Y] + T2[Z]);
	J[Y][Y] = density * (T2[Z] + T2[X]);
	J[Z][Z] = density * (T2[X] + T2[Y]);
	J[X][Y] = J[Y][X] = -density * TP[X];
	J[Y][Z] = J[Z][Y] = -density * TP[Y];
	J[Z][X] = J[X][Z] = -density * TP[Z];

	/* translate inertia tensor to center of mass */
	J[X][X] -= mass * (r[Y] * r[Y] + r[Z] * r[Z]);
	J[Y][Y] -= mass * (r[Z] * r[Z] + r[X] * r[X]);
	J[Z][Z] -= mass * (r[X] * r[X] + r[Y] * r[Y]);
	J[X][Y] = J[Y][X] += mass * r[X] * r[Y];
	J[Y][Z] = J[Z][Y] += mass * r[Y] * r[Z];
	J[Z][X] = J[X][Z] += mass * r[Z] * r[X];

	Vector3 iBasis = Vector3(J[X][X], J[Y][X], J[Z][X]);
	Vector3 jBasis = Vector3(J[X][Y], J[Y][Y], J[Z][Y]);
	Vector3 kBasis = Vector3(J[X][Z], J[Y][Z], J[Z][Z]);

	out_tensor = Matrix3(iBasis, jBasis, kBasis);
	return Vector3(r[X], r[Y], r[Z]);
}

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------