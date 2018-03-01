// Authored By: Tom Tsiliopoulos - 100616336
// Authored By: Joss Moo-Young - 100586602

#pragma once
#include "PointHandle.h"
#include "Path.h"
#include <iostream>
#include <fstream>
#include <direct.h>
#include <nfd/src/include/nfd.h>


// Node types
//for all of these node types, m_pos is its position, which always lies on the path.
//todo: template all this stuff
namespace algomath
{
	struct Node
	{
		Node();
		Node(glm::vec3 a_pos);
		virtual ~Node();

		virtual glm::vec3 computePoint(Node* prev, Node* next, Node* afterNext, float tValue) = 0; //goddamn special snowflake catmull-rom thing is the only reason I need 4 Node pointers instead of 1
		virtual std::pair<glm::vec3, glm::vec3> getControlPoints() = 0; //returns 2 vec3s to be used by the node before

		glm::vec3 value;
		std::string name;

		virtual void draw();
	};

	struct LinearNode : public Node
	{
		LinearNode();
		LinearNode(glm::vec3 a_pos);
		~LinearNode() override;

		glm::vec3 computePoint(Node* prev, Node* next, Node* afterNext, float tValue) override;
		std::pair<glm::vec3, glm::vec3> getControlPoints() override;
	};

	struct BezierNode : public Node
	{
		glm::vec3 m_handleRear;
		glm::vec3 m_handleFront;

		BezierNode();
		~BezierNode() override;

		BezierNode(glm::vec3 a_pos, glm::vec3 a_handleRear, glm::vec3 a_handleFront);
		glm::vec3 computePoint(Node* prev, Node* next, Node* afterNext, float tValue) override;
		std::pair<glm::vec3, glm::vec3> getControlPoints() override;

		void draw();
	};

	struct CatmullRomNode : public Node
	{
		CatmullRomNode();
		~CatmullRomNode() override;

		CatmullRomNode(glm::vec3 a_pos);
		glm::vec3 computePoint(Node* prev, Node* next, Node* afterNext, float tValue) override;
		std::pair<glm::vec3, glm::vec3> getControlPoints() override;
	};

	//every Node on the path is a point that will be hit, regardless of which kind of node it is, it is on the path. The nodes themselves hold their own control points
	class PathEditor
	{
	public:
		PathEditor();
		~PathEditor();

		glm::vec3 computePoint(int index, float evaluationTime); //computes point at evaluationTime along the local node. 														 //TODO: make one that takes evaluation time along the whole path
		void updateTable(float tolerance);
		void draw();

		std::vector<Node*> m_nodes;
		algomath::Path<glm::vec3> m_table;
	protected:
		
		void recursiveSubdivide(Node* aPrev, Node* a, Node* b, Node* bAfter, std::list<NodeGraphTableEntry<glm::vec3>>::iterator iter_a, std::list<NodeGraphTableEntry<glm::vec3>>::iterator iter_b, std::list<NodeGraphTableEntry<glm::vec3>>& list, float tolerance); // for adaptive sampling algorithm
	};

	class NodeGrapher: public PathEditor // handles basic state management and UI for graphing a path
	{
	public:
		NodeGrapher();
		~NodeGrapher();

		void draw();
		void updateTable();

		void keyboardFunc(unsigned char key);

		void addLinear(glm::vec3 pos);
		void addCatmullRom(glm::vec3 pos);
		void addBezier(glm::vec3 pos);
		void addBezier(glm::vec3 pos, glm::vec3 handleRear, glm::vec3 handleFront);

		algomath::Path<glm::vec3> getPath() const; //returns a copy of the path this graph is using
		float m_sampleTolerance = 0.1f; //for adaptive sampling
		void pop_back();
		void clearNodes();

		bool openGraphFile1f(algomath::Path<float>& graphToWriteTo);

		bool saveGraphFile3f();
		bool openGraphFile3f();
	private:
		bool openGraphFile3f(algomath::Path<glm::vec3>& graphToWriteTo);
		bool saveGraphFile3f(const algomath::Path<glm::vec3>& save);
	};
}
