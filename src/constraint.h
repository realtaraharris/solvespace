class Constraint : public ConstraintBase {
public:
    // See Entity::Entity().
    Constraint() : ConstraintBase({}), disp() {}

    // These define how the constraint is drawn on-screen.
    struct {
        Vector      offset;
        hStyle      style;
    } disp;

    bool IsVisible() const;
    bool IsStylable() const;
    hStyle GetStyle() const;
    bool HasLabel() const;
    std::string Label() const;

    enum class DrawAs { DEFAULT, HOVERED, SELECTED };
    void Draw(DrawAs how, Canvas *canvas);
    Vector GetLabelPos(const Camera &camera);
    void GetReferencePoints(const Camera &camera, std::vector<Vector> *refs);

    void DoLayout(DrawAs how, Canvas *canvas,
                  Vector *labelPos, std::vector<Vector> *refs);
    void DoLine(Canvas *canvas, Canvas::hStroke hcs, Vector a, Vector b);
    void DoStippledLine(Canvas *canvas, Canvas::hStroke hcs, Vector a, Vector b);
    bool DoLineExtend(Canvas *canvas, Canvas::hStroke hcs,
                      Vector p0, Vector p1, Vector pt, double salient);
    void DoArcForAngle(Canvas *canvas, Canvas::hStroke hcs,
                       Vector a0, Vector da, Vector b0, Vector db,
                       Vector offset, Vector *ref, bool trim, Vector explodeOffset);
    void DoArrow(Canvas *canvas, Canvas::hStroke hcs,
                 Vector p, Vector dir, Vector n, double width, double angle, double da);
    void DoLineWithArrows(Canvas *canvas, Canvas::hStroke hcs,
                          Vector ref, Vector a, Vector b, bool onlyOneExt);
    int  DoLineTrimmedAgainstBox(Canvas *canvas, Canvas::hStroke hcs,
                                 Vector ref, Vector a, Vector b, bool extend,
                                 Vector gr, Vector gu, double swidth, double sheight);
    int  DoLineTrimmedAgainstBox(Canvas *canvas, Canvas::hStroke hcs,
                                 Vector ref, Vector a, Vector b, bool extend = true);
    void DoLabel(Canvas *canvas, Canvas::hStroke hcs,
                 Vector ref, Vector *labelPos, Vector gr, Vector gu);
    void DoProjectedPoint(Canvas *canvas, Canvas::hStroke hcs, Vector *p);
    void DoProjectedPoint(Canvas *canvas, Canvas::hStroke hcs, Vector *p, Vector n, Vector o);

    void DoEqualLenTicks(Canvas *canvas, Canvas::hStroke hcs,
                         Vector a, Vector b, Vector gn, Vector *refp);
    void DoEqualRadiusTicks(Canvas *canvas, Canvas::hStroke hcs,
                            hEntity he, Vector *refp);

    std::string DescriptionString() const;

    bool ShouldDrawExploded() const;

    static hConstraint AddConstraint(Constraint *c, bool rememberForUndo = true);
    static void MenuConstrain(Command id);
    static void DeleteAllConstraintsFor(Constraint::Type type, hEntity entityA, hEntity ptA);

    static hConstraint ConstrainCoincident(hEntity ptA, hEntity ptB);
    static hConstraint Constrain(Constraint::Type type, hEntity ptA, hEntity ptB, hEntity entityA,
                                 hEntity entityB = Entity::NO_ENTITY, bool other = false,
                                 bool other2 = false);
    static hConstraint TryConstrain(Constraint::Type type, hEntity ptA, hEntity ptB,
                                    hEntity entityA, hEntity entityB = Entity::NO_ENTITY,
                                    bool other = false, bool other2 = false);
    static bool ConstrainArcLineTangent(Constraint *c, Entity *line, Entity *arc);
    static bool ConstrainCubicLineTangent(Constraint *c, Entity *line, Entity *cubic);
    static bool ConstrainCurveCurveTangent(Constraint *c, Entity *eA, Entity *eB);
};
