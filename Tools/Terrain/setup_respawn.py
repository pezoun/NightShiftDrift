"""Connect the existing respawn component to all roads without fake links."""
import unreal as u, json
E=u.get_editor_subsystem(u.EditorActorSubsystem)
world=u.get_editor_subsystem(u.UnrealEditorSubsystem).get_editor_world()
assert '/L_Touge_Region.' in world.get_path_name()
root=u.Paths.convert_relative_path_to_full(u.Paths.project_dir())
p=json.load(open(root+'SourceArt/Terrain/region_manifest.json'))['road_centerlines_m']
w,r,v,s=[p[k] for k in ['WestPass','EastPass','ValleyLink','SouthLink']]
def nearest(points,target):return min(range(len(points)),key=lambda i:sum((points[i][j]-target[j])**2 for j in range(3)))
ic=nearest(v,s[-1]);iw=nearest(w,v[0]);ir=nearest(r,v[-1])
traversal=w+r[1:]+s[1:]+list(reversed(v[:ic+1]))[1:]+list(reversed(w[:iw+1]))[1:]
traversal+=list(reversed(r[ir:]))[1:]+list(reversed(v[ic:]))[1:]+list(reversed(s))[1:]
label='RoadNetwork_RespawnSpline'
found=[a for a in E.get_all_level_actors() if a.get_actor_label()==label]
a=found[0] if found else E.spawn_actor_from_class(u.EditorAssetLibrary.load_blueprint_class('/Game/Vehicles/Blueprints/BP_TrackRespawnSpline'),u.Vector())
a.set_actor_label(label);a.set_folder_path('03_Gameplay');a.set_editor_property('tags',['TrackSpline'])
sp=a.get_component_by_class(u.SplineComponent)
assert sp,'Respawn blueprint has no spline component'
sp.clear_spline_points(False)
for i,point in enumerate(traversal):
    sp.add_spline_point(u.Vector(*(x*100 for x in point)),u.SplineCoordinateSpace.WORLD,False)
    sp.set_spline_point_type(i,u.SplinePointType.LINEAR,False)
sp.set_closed_loop(False,False);sp.update_spline()
try:sp.set_editor_property('bSplineHasBeenEdited',True)
except Exception:pass
point_count=sp.get_number_of_spline_points()
assert u.get_editor_subsystem(u.LevelEditorSubsystem).save_current_level()
print('TOUGE_RESPAWN_READY',point_count)
