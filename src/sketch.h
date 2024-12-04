#ifndef SKETCH_H
#define SKETCH_H

class Sketch {
  public:
  // these are user-editable, and define the sketch
  IdList<Group, hGroup>           group;
  List<hGroup>                    groupOrder;
  IdList<Constraint, hConstraint> constraint;
  IdList<Request, hRequest>       request;
  IdList<Style, hStyle>           style;

  // these are generated from the above
  IdList<Entity, hEntity> entity;
  IdList<Param, hParam>   param;

  inline Constraint *GetConstraint (hConstraint h) { return constraint.FindById (h); }
  inline Entity     *GetEntity (hEntity h) { return entity.FindById (h); }
  inline Param      *GetParam (hParam h) { return param.FindById (h); }
  inline Request    *GetRequest (hRequest h) { return request.FindById (h); }
  inline Group      *GetGroup (hGroup h) { return group.FindById (h); }

  // styles are handled a bit differently
  void Clear ();

  BBox   CalculateEntityBBox (bool includingInvisible);
  Group *GetRunningMeshGroupFor (hGroup h);
};

#endif // SKETCH_H