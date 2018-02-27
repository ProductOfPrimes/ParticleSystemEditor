#include <boost/serialization/access.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "NodeGrapher.h"
#include "GLUT/glut.h"
#include "TTK\GraphicsUtils.h"
#include "..\include\NodeGrapher.h"


#define MIN(val,min) (val > min) ? min : val
#define MAX(val,max) (val < max) ? max : val

namespace algomath
{
	const static glm::mat4x4 s_CATMULLROM_MATRIX = glm::mat4x4
	(
		-0.5, 1.5, -1.5, .5,
		1., -2.5, 2., -0.5,
		-0.5, 0., 0.5, 0.,
		0., 1., 0., 0.
	);

	const static glm::mat4x4 s_BEZIER_SPLINE_MATRIX = glm::mat4x4
	(
		-1., 3., -3, 1.,
		3., -6., 3., 0.,
		-3., 3., 0., 0.,
		1., 0., 0., 0.
	);

	Node::Node()
	{
	}
	Node::Node(glm::vec3 a_pos) : value(a_pos)
	{
	}

	Node::~Node()
	{
	}

	void Node::draw()
	{
		glm::vec4 colour = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		TTK::Graphics::DrawCube(value, 0.5f, colour);
	}

	PathEditor::PathEditor()
	{
	}

	PathEditor::~PathEditor()
	{
	}

	glm::vec3 PathEditor::computePoint(int index, float localTime)
	{
		return m_nodes[index]->computePoint(m_nodes[MAX((index - 1), 0)], m_nodes[MIN((index + 1), m_nodes.size() - 1)], m_nodes[MIN((index + 2), m_nodes.size() - 1)], localTime);
	}

	void PathEditor::updateTable(float tolerance)
	{
		m_table.m_data.clear();

		for (int i = 0; i < m_nodes.size() - 1; i++) // for each interval...
		{
			//create a subtable of values for that interval with subdivision
			std::list<NodeGraphTableEntry<glm::vec3>> subTable;

			Node* beforefirstNode = m_nodes[MAX(i - 1, 0)]; //because Catmull-Rom is a super freaking special snowflake
			Node* firstNode = m_nodes[i];
			Node* secondNode = m_nodes[i + 1];
			Node* afterSecondNode = m_nodes[MIN(i + 2, m_nodes.size() - 1)];

			NodeGraphTableEntry<glm::vec3> first = NodeGraphTableEntry<glm::vec3>(firstNode->value, 0.0f);
			NodeGraphTableEntry<glm::vec3> second = NodeGraphTableEntry<glm::vec3>(secondNode->value, 1.0f);

			subTable.push_back(first);
			subTable.push_back(second);

			recursiveSubdivide(beforefirstNode, firstNode, secondNode, afterSecondNode, subTable.begin(), --subTable.end(), subTable, tolerance); // takes subTable as a ref parameter to fill it with subdivisions

			//insert subtable into master table
			m_table.m_data.push_back(subTable);
		}
		//compute pairwise distances and lengths along the curve
		m_table.updateDistances();
	}

	void PathEditor::draw()
	{
		glm::vec4 col = glm::vec4(0.9f, 0.4f, 0.4f, 1.0f);

		// Draw Node lines
		for (int i = 0; i < m_table.numIntervals(); i++)
		{
			auto row = &m_table.m_data[i];

			auto it = row->begin();
			auto it2 = std::next(it);

			while (it2 != row->end())
			{
				//TTK::Graphics::DrawLine(it->val, it2->val, 5.5f, col);

				TTK::Graphics::DrawVector(it->val, it2->val - it->val, 5.5f, col);
				//TTK::Graphics::DrawPoint(it->val, 4.0f, col);
				it2++;
				it++;
			}
		}
	}

	void PathEditor::recursiveSubdivide(Node * aPrev, Node * a, Node * b, Node * bAfter, std::list<NodeGraphTableEntry<glm::vec3>>::iterator iter_a, std::list<NodeGraphTableEntry<glm::vec3>>::iterator iter_b, std::list<NodeGraphTableEntry<glm::vec3>>& list, float tolerance)
	{
		float tHalf = (iter_a->t + iter_b->t) / 2.0f;
		glm::vec3 point_ab = a->computePoint(aPrev, b, bAfter, tHalf); // halfway point

		//check if within tolerance i.e. (A + B - C) < tolerance
		glm::vec3 aVec = iter_a->val;
		glm::vec3 bVec = iter_b->val;

		float sideA = glm::length(point_ab - aVec);
		float sideB = glm::length(point_ab - bVec);
		float sideC = glm::length(bVec - aVec);

		if (sideA + sideB - sideC < tolerance)
		{
			return;
		}
		else
		{
			//insert new element into list
			NodeGraphTableEntry<glm::vec3>& newEntry = NodeGraphTableEntry<glm::vec3>(point_ab, tHalf); //tHalf, point_ab.x, point_ab.y);
			newEntry.val = point_ab;

			std::list<NodeGraphTableEntry<glm::vec3>>::iterator iter_ab = (list.insert(iter_b, newEntry));
			recursiveSubdivide(aPrev, a, b, bAfter, iter_a, iter_ab, list, tolerance);
			recursiveSubdivide(aPrev, a, b, bAfter, iter_ab, iter_b, list, tolerance);
		}
	}

	LinearNode::LinearNode() : Node()
	{
	}

	LinearNode::LinearNode(glm::vec3 a_pos) : Node(a_pos)
	{
	}

	LinearNode::~LinearNode()
	{
	}

	glm::vec3 LinearNode::computePoint(Node* prev, Node* next, Node* afterNext, float tValue)
	{
		return lerp(value, next->value, tValue);
	}

	std::pair<glm::vec3, glm::vec3> LinearNode::getControlPoints()
	{
		return std::pair<glm::vec3, glm::vec3>(value, value);
	}

	BezierNode::BezierNode() : Node()
	{
	}

	BezierNode::~BezierNode()
	{
	}

	BezierNode::BezierNode(glm::vec3 a_pos, glm::vec3 a_handleRear, glm::vec3 a_handleFront) : Node(a_pos), m_handleRear(a_handleRear), m_handleFront(a_handleFront)
	{
	}

	glm::vec3 BezierNode::computePoint(Node* prev, Node * targetNode, Node* afterNext, float tValue)
	{
		std::pair<glm::vec3, glm::vec3> cp = targetNode->getControlPoints();

		glm::vec3 a = value;
		glm::vec3 b = m_handleFront;
		glm::vec3 c = cp.first;
		glm::vec3 d = cp.second;

		float coeff1 = pow((1.0f - tValue), 3);
		float coeff2 = tValue * 3.0f * pow((1.0f - tValue), 2);
		float coeff3 = (3.0f * pow(tValue, 2)) * (1.0f - tValue);
		float coeff4 = pow(tValue, 3);

		return coeff1 * a + coeff2 * b + coeff3 * c + coeff4 * d;
	}

	std::pair<glm::vec3, glm::vec3> BezierNode::getControlPoints()
	{
		return std::pair<glm::vec3, glm::vec3>(m_handleRear, value);
	}

	void BezierNode::draw()
	{
		glm::vec4 colour = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		TTK::Graphics::DrawCube(value, 1.0f, colour);

		colour = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
		TTK::Graphics::DrawCube(m_handleRear, 0.3f, colour);
		TTK::Graphics::DrawCube(m_handleFront, 0.3f, colour);
	}

	NodeGrapher::NodeGrapher()
	{
	}

	NodeGrapher::~NodeGrapher()
	{
		clearNodes();
	}

	void NodeGrapher::addLinear(glm::vec3 pos)
	{
		LinearNode* newNode = new LinearNode(pos);

		m_nodes.push_back(newNode);
	}

	void NodeGrapher::draw()
	{
		for (int i = 0; i < m_nodes.size(); i++)
		{
			m_nodes[i]->draw();
		}
		PathEditor::draw();
	}

	void NodeGrapher::updateTable()
	{
		PathEditor::updateTable(m_sampleTolerance);
	}

	void NodeGrapher::addBezier(glm::vec3 pos)
	{
		BezierNode* newNode = new BezierNode(pos, pos, pos);
		m_nodes.push_back(newNode);
	}

	void NodeGrapher::addBezier(glm::vec3 pos, glm::vec3 handleRear, glm::vec3 handleFront)
	{
		BezierNode* newNode = new BezierNode(pos, handleRear, handleFront);
		m_nodes.push_back(newNode);
	}

	algomath::Path<glm::vec3> NodeGrapher::getPath() const
	{
		return m_table;
	}

	void NodeGrapher::pop_back()
	{
		//detect if it's a bezier. if it is i need to remove its handles too.
		auto node = m_nodes.back();
		delete node;
		m_nodes.pop_back();
	}

	void NodeGrapher::clearNodes()
	{
		int nodes = m_nodes.size();
		for (int i = 0; i < nodes; i++)
		{
			pop_back();
		}
	}

	std::string GetProjectDirectory()
{
	char* cwd = _getcwd(0, 0); // **** microsoft specific ****
	std::string working_directory(cwd);
	std::free(cwd);
	return working_directory;
}

	bool NodeGrapher::openGraphFile1f(algomath::Path<float>& graphToWriteTo) {
		const nfdchar_t *defaultPath = NULL;
		nfdchar_t *outPath = NULL;

		std::string currentDirectory = GetProjectDirectory();
		defaultPath = currentDirectory.c_str();

		nfdresult_t result = NFD_OpenDialog("dat", defaultPath, &outPath);


		if (result == NFD_OKAY)
		{
			
			std::ifstream graphFile(outPath, std::ios::in, std::ios::binary);
			boost::archive::text_iarchive ia(graphFile);

			std::vector<std::vector<NodeGraphTableEntry<float>>> nodeGraphTableEntries; // packed data
			ia >> nodeGraphTableEntries;

			algomath::Path<float> returnGraph; // the actual object to return

			for (size_t i = 0; i < nodeGraphTableEntries.size(); i++)
			{
				std::list<NodeGraphTableEntry<float>> table;

				for each (NodeGraphTableEntry<float> tableEntry in nodeGraphTableEntries[i])
				{
					table.push_back(tableEntry);
				}
				returnGraph.m_data.push_back(table);
			}

			graphFile.close();

			graphToWriteTo = returnGraph;
			return true;
		}
		return false;
	}

	bool NodeGrapher::openGraphFile3f(algomath::Path<glm::vec3>& graphToWriteTo) {
		const nfdchar_t *defaultPath = NULL;
		nfdchar_t *outPath = NULL;

		std::string currentDirectory = GetProjectDirectory();
		defaultPath = currentDirectory.c_str();

		nfdresult_t result = NFD_OpenDialog("dat", defaultPath, &outPath);


		if (result == NFD_OKAY)
		{

			std::ifstream graphFile(outPath, std::ios::in, std::ios::binary);
			boost::archive::text_iarchive ia(graphFile);

			std::vector<std::vector<NodeGraphTableEntry<glm::vec3>>> nodeGraphTableEntries; // packed data
			ia >> nodeGraphTableEntries;

			algomath::Path<glm::vec3> returnGraph; // the actual object to return

			for (size_t i = 0; i < nodeGraphTableEntries.size(); i++)
			{
				std::list<NodeGraphTableEntry<glm::vec3>> table;
				for each (NodeGraphTableEntry<glm::vec3> tableEntry in nodeGraphTableEntries[i])
				{
					table.push_back(tableEntry);
				}
				returnGraph.m_data.push_back(table);
			}
			returnGraph.updateDistances();

			graphFile.close();

			graphToWriteTo = returnGraph;
			return true;
		}
		return false;
	}

	bool NodeGrapher::saveGraphFile3f()
	{
		return saveGraphFile3f(m_table);
		return 0;
	}

	bool NodeGrapher::openGraphFile3f()
	{
		return openGraphFile3f(m_table);
		return 0;
	}

	bool NodeGrapher::saveGraphFile3f(const Path<glm::vec3>& saveThis) {
		const nfdchar_t *defaultPath = NULL;
		nfdchar_t *outPath = NULL;

		std::string currentDirectory = GetProjectDirectory();
		defaultPath = currentDirectory.c_str();

		nfdresult_t result = NFD_SaveDialog("dat", defaultPath, &outPath);

		if (result == NFD_OKAY)
		{
			std::ofstream graphFile(outPath, std::ios::out, std::ios::binary);
			boost::archive::text_oarchive oa(graphFile);
			std::vector<std::vector<NodeGraphTableEntry<glm::vec3>>> nodeGraphTableEntries; // data to output

			for (int i = 0; i < saveThis.m_data.size(); i++)
			{
				std::vector<NodeGraphTableEntry<glm::vec3>> table;
				for (auto iter = saveThis.m_data[i].begin(); iter != saveThis.m_data[i].end(); iter++)
				{
					table.push_back(*iter);
				}
				nodeGraphTableEntries.push_back(table);
			}

			oa << nodeGraphTableEntries;
			graphFile.close();
			return true;
		}
		return false;
	}


	void NodeGrapher::addCatmullRom(glm::vec3 pos)
	{
		CatmullRomNode* newNode = new CatmullRomNode(pos);
		m_nodes.push_back(newNode);
	}

	void NodeGrapher::keyboardFunc(unsigned char key)
	{
		switch (key)
		{

		case 0x0D: // enter key
			updateTable();
			break;

		case '+': // plus
				  //case '=':
			m_sampleTolerance = MIN(m_sampleTolerance * 1.1f, 9999999.f);
			updateTable();
			break;

		case '-': // minus
				  //case '_':
			m_sampleTolerance = MAX(m_sampleTolerance / 1.1f, 0.000001f);
			updateTable();
			break;

		case 0x08: // backspace
			if (m_nodes.size() > 1)
			{
				pop_back();
				updateTable();
			}
			break;
		}
	}

	CatmullRomNode::CatmullRomNode() : Node()
	{
	}
	CatmullRomNode::~CatmullRomNode()
	{
	}
	CatmullRomNode::CatmullRomNode(glm::vec3 a_pos) : Node(a_pos)
	{
	}
	glm::vec3 CatmullRomNode::computePoint(Node* prev, Node * next, Node* afterNext, float tValue)
	{
		std::pair<glm::vec3, glm::vec3> cp = next->getControlPoints();
		//uses formula uMp

		//create u part
		glm::vec4 u = glm::vec4(tValue * tValue * tValue, tValue * tValue, tValue, 1.f); //t^3, t^2, t, 1

																						 // create p part
		glm::mat4x3 p = glm::mat4x3(
			prev->value.x, prev->value.y, prev->value.z,
			value.x, value.y, value.z,
			cp.second.x, cp.second.y, cp.second.z,
			afterNext->value.x, afterNext->value.y, afterNext->value.z
		);
		return p * s_CATMULLROM_MATRIX * u;
	}


	std::pair<glm::vec3, glm::vec3> CatmullRomNode::getControlPoints()
	{
		return std::pair<glm::vec3, glm::vec3>(value, value);
	}
}