"""Run inside Unreal Python after importing region.r16 into L_Touge_Region."""
import unreal as u
E=u.get_editor_subsystem(u.EditorActorSubsystem)
L=u.get_editor_subsystem(u.LevelEditorSubsystem)
world=u.get_editor_subsystem(u.UnrealEditorSubsystem).get_editor_world()
assert '/L_Touge_Region.' in world.get_path_name(), 'Wrong map; no changes applied'
landscapes=[a for a in E.get_all_level_actors() if isinstance(a,u.Landscape)]
assert len(landscapes)==1, 'Expected one native Landscape'
land=landscapes[0]
land.set_actor_location(u.Vector(-201600,-201600,0),False,False)
land.set_actor_label('Touge Region - native Landscape 4032m')
land.set_folder_path('01_Topography')
terrain='/Game/NightshiftDrift/Materials/MI_TougeLandscape'
if not u.EditorAssetLibrary.does_asset_exist(terrain):
    terrain='/Game/MWLandscapeAutoMaterial/Materials/Landscape/MTL_MWAM_Landscape_MountainRangeExample'
land.set_editor_property('landscape_material',u.load_asset(terrain))

def actor(cls,label,location=(0,0,0),rotation=(0,0,0)):
    found=[a for a in E.get_all_level_actors() if a.get_actor_label()==label]
    if found:return found[0]
    a=E.spawn_actor_from_class(cls,u.Vector(*location),u.Rotator(*rotation))
    a.set_actor_label(label);a.set_folder_path('02_Lighting');return a

sun=actor(u.DirectionalLight,'Touge Sun',(0,0,120000),(-35,-40,0))
sun.light_component.set_editor_property('intensity',5)
sun.light_component.set_editor_property('atmosphere_sun_light',True)
sky=actor(u.SkyLight,'Touge Skylight')
sky.light_component.set_editor_property('real_time_capture',True)
actor(u.SkyAtmosphere,'Touge Atmosphere')
fog=actor(u.ExponentialHeightFog,'Touge Valley Haze',(0,0,2500))
fc=fog.get_component_by_class(u.ExponentialHeightFogComponent)
fc.set_editor_property('fog_density',.006)
fc.set_editor_property('fog_height_falloff',.18)
start=actor(u.PlayerStart,'Touge Trailhead PlayerStart',(-95000,-125000,6700),(0,53,0))
start.set_folder_path('03_Gameplay')
assert L.save_current_level(),'Failed to save native landscape map'
print('TOUGE_REGION_READY',land.get_actor_bounds(False))
