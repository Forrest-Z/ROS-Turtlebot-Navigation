#ifndef ROAD_MAP_HPP
#define ROAD_MAP_HPP
/// \file
/// \brief Probabilisitc road maps


#include <vector>
#include <iosfwd>
#include <unordered_set>

#include <rigid2d/utilities.hpp>
#include <rigid2d/rigid2d.hpp>



namespace planner
{
  using rigid2d::sampleUniformDistribution;
  using rigid2d::Vector2D;
  using rigid2d::euclideanDistance;


  typedef std::vector<Vector2D> polygon;    // convex polygon, vertices counter-clockwise
  typedef std::vector<polygon> obstacle_map;     // constain all polygons in Cspace


  /// \brief Egde connecting two nodes
  struct Edge
  {
    int id = -1;                    // id of node edge connects to
    double d = 0.0;                 // length of edge
  };


  /// \brief Node in the road map
  struct Node
  {
    int id = -1;                            // nodes id, index in nodes
    Vector2D point;                         // (x,y) location in world
    std::vector<Edge> edges;                // edges to other nodes
    std::unordered_set<int> id_set;         // id of adjacent nodes

    /// \brief Checks if edge exists in the id_map
    /// \param id - id of node connected to
    /// \return true if node is connected to id
    bool edgeExists(int id)
    {
      const auto search = id_set.find(id);
      return (search == id_set.end()) ? false : true;
    }
  };


  /// \brief Useful information for determining the closest point to
  ///        a line segment
  struct ClosePoint
  {
    double t = 0.0;           // if t [0, 1] then the min distance is on the line segment
    double sign_d = 0.0;      // signed distance from line to p, d > 0 if on left side in direction p1 => p2
    Vector2D p;               // closest point to line
    bool on_seg = false;      // true if min distance to p is on the segment
  };


  /// \brief Compose shortest distance from linesegment to a point
  ///        p1 and p2 are the bounds of the line segment
  ///        and p3 is the point.
  ///        Checks if the min distance is on the line segment
  /// \param p1 - first bound of line segment
  /// \param p1 - second bound of line segment
  /// \param p3 - point to compose distance to
  /// distance[out] - min distance between line and a point
  /// \returns - true if the closest distance lies on the line segment
  bool minDistLineSegPt(double &distance,
                        const Vector2D &p1,
                        const Vector2D &p2,
                        const Vector2D &p3);

  /// \brief Compose shortest distance from linesegment to a point
  ///        p1 and p2 are the bounds of the line segment
  ///        and p3 is the point.
  ///        Does not check if the min distance is on the line segment
  /// \param p1 - first bound of line segment
  /// \param p1 - second bound of line segment
  /// \param p3 - point to compose distance to
  /// \returns - min distance between line and a point
  double minDistLineSegPt(const Vector2D &p1,
                          const Vector2D &p2,
                          const Vector2D &p3);

  /// \brief Compose shortest distance from linesegment to a point
  ///        p1 and p2 are the bounds of the line segment
  ///        and p3 is the point.
  ///        Checks if the min distance is on the line segment
  /// \param p1 - first bound of line segment
  /// \param p1 - second bound of line segment
  /// \param p3 - point to compose distance to
  /// \returns - data regarding the state of the min distance
  ///            from a point to the line segment
  ClosePoint signMinDist2Line(const Vector2D &p1,
                              const Vector2D &p2,
                              const Vector2D &p3);


  // double minDist(const Vector2D &p1,
  //                         const Vector2D &p2,
  //                         const Vector2D &p3);



   /// \brief Compose graph for global planning
  class RoadMap
  {
  public:
    /// \brief Compose road map
    /// \param xmin - lower x world bound
    /// \param xmax - upper x world bound
    /// \param ymin - lower y world bound
    /// \param ymax - upper y world bound
    /// \param bnd_rad - bounding radius around robot
    /// \param neighbors - number of closest neighbors examine for each configuration
    /// \param num_nodes - number of nodes to put in road map
    RoadMap(double xmin, double xmax,
            double ymin, double ymax,
            double bnd_rad,
            unsigned int neighbors, unsigned int num_nodes,
            obstacle_map obs_map);


    /// \brief Retreive the graph
    /// roadmap[out] - the graph
    void getRoadMap(std::vector<Node> &roadmap) const;

    /// \brief Print road map
    void printRoadMap() const;

    /// \brief Construct the roadmap
    /// \param start - start configuration
    /// \param goal - goal configuration
    void constructRoadMap(const Vector2D &start, const Vector2D &goal);

    /// \brief Check whether a point is in free space
    /// \param q - random configuration
    /// \return - true if free space
    bool isFreeSpace(const Vector2D &q) const;

    /// \brief Check whether an edge between two nodes is feasible
    /// \param p1 - first bound of edge
    /// \param p1 - second bound of edge
    /// \return - true no collision between edge on polygons
    bool straightLinePath(const Vector2D &p1, const Vector2D &p2) const;


    /// \brief Check whether a point isnside or close to a polygon
    /// \param poly - plygon to examine
    /// \param q - random configuration
    /// \return - true if inside or close to polygon
    bool ptInsidePolygon(const polygon &poly, const Vector2D &q) const;

    /// \brief Check whether line segment intersects a polygon
    /// \param poly - plygon to examine
    /// \param p1 - first bound of line segment
    /// \param p1 - second bound of line segment
    /// \return - true if intersects polygon
    bool lnSegIntersectPolygon(const polygon &poly,
                               const Vector2D &p1,
                               const Vector2D &p2) const;


  private:
    /// \brief Finds K nearest neighbors in roadmap
    /// \param query - query node
    /// neighbors[out] - index on neighbors
    void nearestNeighbors(const Node &query, std::vector<int> &neighbors) const;

    /// \brief Check if node collides with boundaries of map
    /// \param q - the (x, y) location of a node
    /// \returns - true if q collides with boundary
    bool collideWalls(const Vector2D &q) const;

    /// \brief Check whether line segment comes within
    ///        a distance theshold of a polygon
    /// \param poly - plygon to examine
    /// \param p1 - first bound of line segment
    /// \param p1 - second bound of line segment
    /// \return - true if line segment is within tolerance of a polygon
    bool lnSegClose2Polygon(const polygon &poly,
                            const Vector2D &p1,
                            const Vector2D &p2) const;

    /// \brief Insert a node into roadmap
    /// \param q - the (x, y) location of a node
    void addNode(const Vector2D &q);

    /// \brief Add edge between two nodes
    /// \param id1 - key of first node
    /// \param id2 - key of second node
    void addEdge(int id1, int id2);

    /// \brief Generate random point in world
    Vector2D randomPoint() const;


    double xmin, xmax, ymin, ymax;      // bounds of the world
    double bnd_rad;                     // distance threshold between nodes/path from obstacles
    unsigned int k, n;                  // number of closest neighbors, number of nodes
    obstacle_map obs_map;               // collection of all the polygons
    std::vector<Node> nodes;            // graph representation of road map

  };





} // end namespace




#endif
