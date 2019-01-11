/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   Copyright (c) 2013-2019 The plumed team
   (see the PEOPLE file at the root of the distribution for a list of names)

   See http://www.plumed.org for more information.

   This file is part of plumed, version 2.

   plumed is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   plumed is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with plumed.  If not, see <http://www.gnu.org/licenses/>.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
#include "PathBase.h"
#include "core/ActionRegister.h"

//+PLUMEDOC COLVAR PATH
/*
Path collective variables with a more flexible framework for the distance metric being used.

The Path Collective Variables developed by Branduardi and co-workers \cite brand07 allow one
to compute the progress along a high-dimensional path and the distance from the high-dimensional
path.  The progress along the path (s) is computed using:

\f[
s = \frac{ \sum_{i=1}^N i \exp( -\lambda R[X - X_i] ) }{ \sum_{i=1}^N \exp( -\lambda R[X - X_i] ) }
\f]

while the distance from the path (z) is measured using:

\f[
z = -\frac{1}{\lambda} \ln\left[ \sum_{i=1}^N \exp( -\lambda R[X - X_i] ) \right]
\f]

In these expressions \f$N\f$ high-dimensional frames (\f$X_i\f$) are used to describe the path in the high-dimensional
space. The two expressions above are then functions of the distances from each of the high-dimensional frames \f$R[X - X_i]\f$.
Within PLUMED there are multiple ways to define the distance from a high-dimensional configuration.  You could calculate
the RMSD distance or you could calculate the ammount by which a set of collective variables change.  As such this implementation
of the path cv allows one to use all the difference distance metrics that are discussed in \ref dists. This is as opposed to
the alternative implementation of path (\ref PATHMSD) which is a bit faster but which only allows one to use the RMSD distance.

The \f$s\f$ and \f$z\f$ variables are calculated using the above formulas by default.  However, there is an alternative method
of calculating these collective variables, which is detailed in \cite bernd-path.  This alternative method uses the tools of
geometry (as opposed to algebra, which is used in the equations above).  In this alternative formula the progress along the path
\f$s\f$ is calculated using:

\f[
s = i_2 + \textrm{sign}(i_2-i_1) \frac{ \sqrt{( \mathbf{v}_1\cdot\mathbf{v}_2 )^2 - |\mathbf{v}_3|^2(|\mathbf{v}_1|^2 - |\mathbf{v}_2|^2) } }{2|\mathbf{v}_3|^2} - \frac{\mathbf{v}_1\cdot\mathbf{v}_3 - |\mathbf{v}_3|^2}{2|\mathbf{v}_3|^2}
\f]

where \f$\mathbf{v}_1\f$ and \f$\mathbf{v}_3\f$ are the vectors connecting the current position to the closest and second closest node of the path,
respectfully and \f$i_1\f$ and \f$i_2\f$ are the projections of the closest and second closest frames of the path. \f$\mathbf{v}_2\f$, meanwhile, is the
vector connecting the closest frame to the second closest frame.  The distance from the path, \f$z\f$ is calculated using:

\f[
z = \sqrt{ \left[ |\mathbf{v}_1|^2 - |\mathbf{v}_2| \left( \frac{ \sqrt{( \mathbf{v}_1\cdot\mathbf{v}_2 )^2 - |\mathbf{v}_3|^2(|\mathbf{v}_1|^2 - |\mathbf{v}_2|^2) } }{2|\mathbf{v}_3|^2} - \frac{\mathbf{v}_1\cdot\mathbf{v}_3 - |\mathbf{v}_3|^2}{2|\mathbf{v}_3|^2} \right) \right]^2 }
\f]

The symbols here are as they were for \f$s\f$.  If you would like to use these equations to calculate \f$s\f$ and \f$z\f$ then you should use the GPATH flag.
The values of \f$s\f$ and \f$z\f$ can then be referenced using the gspath and gzpath labels.

\par Examples

In the example below the path is defined using RMSD distance from frames.
The reference frames in the path are defined in the pdb file.  In this frame
each configuration in the path is separated by a line containing just the word END.

\plumedfile
p1: PATH REFERENCE=file.pdb TYPE=OPTIMAL LAMBDA=500.0
PRINT ARG=p1.sss,p1.zzz STRIDE=1 FILE=colvar FMT=%8.4f
\endplumedfile

In the example below the path is defined using the values of two torsional angles (t1 and t2).
In addition, the \f$s\f$ and \f$z\f$ are calculated using the geometric expressions described
above rather than the alegebraic expressions that are used by default.

\plumedfile
t1: TORSION ATOMS=5,7,9,15
t2: TORSION ATOMS=7,9,15,17
pp: PATH TYPE=EUCLIDEAN REFERENCE=epath.pdb GPATH NOSPATH NOZPATH
PRINT ARG=pp.* FILE=colvar
\endplumedfile

Notice that the LAMBDA parameter is not required here as we are not calculating \f$s\f$ and \f$s\f$
using the algebraic formulae defined earlier.  The positions of the frames in the path are defined
in the file epath.pdb.  An extract from this file looks as shown below.

\verbatim
REMARK ARG=t1,t2 t1=-4.25053  t2=3.88053
END
REMARK ARG=t1,t2 t1=-4.11     t2=3.75
END
REMARK ARG=t1,t2 t1=-3.96947  t2=3.61947
END
\endverbatim

The remarks in this pdb file tell PLUMED the labels that are being used to define the position in the
high dimensional space and the values that these arguments have at each point on the path.

The following input instructs PLUMED to calculate the values of the path collective variables.  The frames that make up this
path are defined in the file all.pdb and all distances are measured using the OPTIMAL metric that is discussed in the manual
page on \ref RMSD.

\plumedfile
p2: PATH REFERENCE=all.pdb LAMBDA=69087
PRINT ARG=p2.spath,p2.zpath STRIDE=1 FILE=colvar
\endplumedfile

If you wish to use collective variable values in the definition of your path you would use an input file with something like this:

\plumedfile
d1: DISTANCE ATOMS=1,2
d2: DISTANCE ATOMS=3,4a
p2: PATH REFERENCE=mypath.pdb LAMBDA=2 TYPE=EUCLIDEAN
PRINT ARG=p2.spath,p2.zpath STRIDE=1 FILE=colvar
\endplumedfile

The corresponding pdb file containing the  definitions of the frames in the path would then look like this:

\verbatim
DESCRIPTION: a defintiion of a PATH
REMARK TYPE=EUCLIDEAN
REMARK ARG=d1,d2
REMARK d1=1.0 d2=1.0
END
REMARK TYPE=EUCLIDEAN
REMARK ARG=d1,d2
REMARK d1=2.0 d2=2.0
END
\endverbatim

For each frame in the path you must specify the arguments that should be used to calculate the distance between the instantaneous configuration
of the system and the reference configurations together with the values that these arguments take in each of the reference configurations.

*/
//+ENDPLUMEDOC

namespace PLMD {
namespace mapping {

class Path : public PathBase {
public:
  static void registerKeywords( Keywords& keys );
  explicit Path(const ActionOptions&);
};

PLUMED_REGISTER_ACTION(Path,"PATH")

void Path::registerKeywords( Keywords& keys ) {
  PathBase::registerKeywords( keys ); keys.remove("PROPERTY");
  keys.addFlag("NOSPATH",false,"do not calculate the spath position");
  keys.remove("LOWMEM"); keys.use("GPATH");
}

Path::Path(const ActionOptions& ao):
  Action(ao),
  PathBase(ao)
{
  setLowMemOption( true );
  bool nos; parseFlag("NOSPATH",nos);

  std::string empty;
  if(!nos) {
    if( getPropertyIndex("spath")!=0 || getNumberOfProperties()>1 ) {
      error("paths only work when there is a single property called sss being calculated");
    }
    if( getLambda()==0 ) error("you must set LAMBDA parameter in order to calculate spath position.  Use LAMBDA/NOSPATH keyword");
    empty="LABEL=spath";
    addVessel("SPATH",empty,0);
  }
  readVesselKeywords();
  checkRead();
}

}
}
