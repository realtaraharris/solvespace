// The format for entities stored on the clipboard.
class ClipboardRequest {
public:
    Request::Type type;
    int         extraPoints;
    hStyle      style;
    std::string str;
    std::string font;
    Platform::Path file;
    bool        construction;

    Vector      point[MAX_POINTS_IN_ENTITY] = {
	    Vector(0, 0, 0),
		Vector(0, 0, 0),
		Vector(0, 0, 0),

		Vector(0, 0, 0),
		Vector(0, 0, 0),
		Vector(0, 0, 0),

		Vector(0, 0, 0),
		Vector(0, 0, 0),
		Vector(0, 0, 0),

		Vector(0, 0, 0),
		Vector(0, 0, 0),
		Vector(0, 0, 0)
	};
    double      distance;

    hEntity     oldEnt;
    hEntity     oldPointEnt[MAX_POINTS_IN_ENTITY];
    hRequest    newReq;
};