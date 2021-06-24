#include "srpch.h"
#include "MeshRendering.h"
#include "Triangulation.h"

//#include <igl/opengl/glfw/Viewer.h>

namespace sunrise::math::mesh {

	
	void displayMesh(Mesh& mesh)
	{
		//todo:  displayMesh(Mesh& mesh) not implimented anymore
		SR_ERROR("displayMesh(Mesh& mesh) not implimented yet. Nothing will hapen");
			/*igl::opengl::glfw::Viewer viewer;

			Eigen::MatrixXd verts;
			Eigen::MatrixXi indicies;


			makeLibiglMesh(mesh, 0, verts, indicies);

			viewer.data().set_mesh(verts, indicies);



			for (size_t i = 1; i < mesh.indicies.size(); i++)
			{
				viewer.append_mesh(true);

				Eigen::MatrixXd verts2;
				Eigen::MatrixXi indicies2;

				makeLibiglMesh(mesh, i, verts2, indicies2);

				viewer.data().set_mesh(verts2, indicies2);
			}

			viewer.launch();*/

	}
	


}