"""Unreal Editor Python: road ribbons on the region's graded corridors."""
import unreal as u, math, os, json, importlib.util
ROOT=u.Paths.convert_relative_path_to_full(u.Paths.project_dir())
spec=importlib.util.spec_from_file_location('touge_region',ROOT+'Tools/Terrain/generate_region.py')
region=importlib.util.module_from_spec(spec);spec.loader.exec_module(region)
E=u.get_editor_subsystem(u.EditorActorSubsystem)
L=u.get_editor_subsystem(u.LevelEditorSubsystem)
world=u.get_editor_subsystem(u.UnrealEditorSubsystem).get_editor_world()
assert '/L_Touge_Region.' in world.get_path_name(),'Wrong map'
BASE='/Game/NightshiftDrift'
out=ROOT+'Saved/TougeDesign/Roads/'
os.makedirs(out,exist_ok=True)

def mat(name,texture=None,color=None,roughness=.9):
    path=BASE+'/Materials/'+name
    if u.EditorAssetLibrary.does_asset_exist(path):return u.load_asset(path)
    m=u.AssetToolsHelpers.get_asset_tools().create_asset(name,BASE+'/Materials',u.Material,u.MaterialFactoryNew())
    lib=u.MaterialEditingLibrary
    if texture:
        t=lib.create_material_expression(m,u.MaterialExpressionTextureSample)
        t.texture=u.load_asset(texture)
        if 'Asphalt' in name:
            mult=lib.create_material_expression(m,u.MaterialExpressionMultiply)
            tint=lib.create_material_expression(m,u.MaterialExpressionConstant);tint.r=.22
            lib.connect_material_expressions(t,'RGB',mult,'A')
            lib.connect_material_expressions(tint,'',mult,'B')
            lib.connect_material_property(mult,'',u.MaterialProperty.MP_BASE_COLOR)
            normal=lib.create_material_expression(m,u.MaterialExpressionTextureSample)
            normal.texture=u.load_asset('/Game/Roads/Asphalt024A_2K-JPG_NormalDX')
            normal.set_editor_property('samplerType',u.MaterialSamplerType.SAMPLERTYPE_NORMAL)
            lib.connect_material_property(normal,'RGB',u.MaterialProperty.MP_NORMAL)
        else:
            lib.connect_material_property(t,'RGB',u.MaterialProperty.MP_BASE_COLOR)
    else:
        t=lib.create_material_expression(m,u.MaterialExpressionConstant3Vector)
        t.constant=u.LinearColor(*color,1)
        lib.connect_material_property(t,'',u.MaterialProperty.MP_BASE_COLOR)
    r=lib.create_material_expression(m,u.MaterialExpressionConstant);r.r=roughness
    lib.connect_material_property(r,'',u.MaterialProperty.MP_ROUGHNESS)
    lib.recompile_material(m)
    return m

terrain_path=BASE+'/Materials/MI_TougeLandscape'
if not u.EditorAssetLibrary.does_asset_exist(terrain_path):
    u.EditorAssetLibrary.duplicate_asset('/Game/MWLandscapeAutoMaterial/Materials/Landscape/MTL_MWAM_Landscape_MountainRangeExample',terrain_path)
terrain=u.load_asset(terrain_path)
u.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(terrain,'MW_SnowWorldPosition',10000000)
u.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(terrain,'MW_SnowMaskMultiplier',0)
for switch in ['MW_UseSnow','MW_[x]_UseGrassProcedurals','MW_[x]_UseDirtProcedurals','MW_[x]_UseStonesProcedurals']:
    u.MaterialEditingLibrary.set_material_instance_static_switch_parameter_value(terrain,switch,False)
u.MaterialEditingLibrary.update_material_instance(terrain)
for a in E.get_all_level_actors():
    if isinstance(a,u.Landscape):a.set_editor_property('landscape_material',terrain)

asphalt=mat('M_TougeAsphaltSurface','/Game/Roads/Asphalt024A_2K-JPG_Color')
shoulder=mat('M_TougeShoulder','/Game/MWLandscapeAutoMaterial/Textures/Ground/TEX_MWAM_Stones_col')
white=mat('M_TougeWhite',color=(.58,.58,.53))
amber=mat('M_TougeAmber',color=(.65,.32,.035))

def ribbon(points,name,left,right,lift):
    vertices=[];uvs=[];faces=[];length=0
    for i,p in enumerate(points):
        q,r=points[max(0,i-1)],points[min(len(points)-1,i+1)]
        dx,dy=r[0]-q[0],r[1]-q[1];d=math.hypot(dx,dy);nx,ny=-dy/d,dx/d
        if i:length+=math.dist(p,points[i-1])
        for off in (left,right):
            vertices.append((100*(p[0]+nx*off),-100*(p[1]+ny*off),100*(p[2]+lift)))
            uvs.append(((off-left)/3,length/3))
        if i:
            a=2*(i-1);faces.extend([(a,a+3,a+2),(a,a+1,a+3)])
    file=out+name+'.obj'
    with open(file,'w') as f:
        f.write('o '+name+'\ns 1\n')
        for p in vertices:f.write('v %.3f %.3f %.3f\n'%p)
        for uv in uvs:f.write('vt %.5f %.5f\n'%uv)
        for face in faces:
            ids=[j+1 for j in face];f.write('f '+' '.join('%d/%d'%(j,j) for j in ids)+'\n')
    task=u.AssetImportTask();task.filename=file;task.destination_path=BASE+'/Roads';task.destination_name=name
    task.automated=True;task.save=True;task.replace_existing=True
    options=u.FbxImportUI();options.import_mesh=True;options.import_materials=False;options.import_textures=False
    options.mesh_type_to_import=u.FBXImportType.FBXIT_STATIC_MESH
    options.static_mesh_import_data.combine_meshes=True
    options.static_mesh_import_data.auto_generate_collision=False
    options.static_mesh_import_data.generate_lightmap_u_vs=False
    task.options=options
    u.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    mesh=u.load_asset(task.imported_object_paths[0])
    body=mesh.get_editor_property('body_setup')
    body.set_editor_property('collision_trace_flag',u.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
    body.set_editor_property('double_sided_geometry',True)
    return mesh

def place(mesh,label,material,folder,collision=True):
    old=[a for a in E.get_all_level_actors() if a.get_actor_label()==label]
    a=old[0] if old else E.spawn_actor_from_class(u.StaticMeshActor,u.Vector())
    a.set_actor_label(label);a.set_folder_path(folder)
    a.static_mesh_component.set_static_mesh(mesh);a.static_mesh_component.set_material(0,material)
    if not collision:a.static_mesh_component.set_collision_enabled(u.CollisionEnabled.NO_COLLISION)
    return a

for name,pts in region.PATHS.items():
    folder='04_RoadNetwork/'+name
    for suffix,l,r,z,material,collide in [
        ('Shoulder',-5.5,5.5,-.08,shoulder,True),
        ('Asphalt',-4,4,0,asphalt,True),
        ('LeftLine',-3.72,-3.60,.02,white,False),
        ('RightLine',3.60,3.72,.02,white,False),
        ('CentreLine',-.065,.065,.02,amber,False)]:
        asset='SM_'+name+'_'+suffix
        place(ribbon(pts,asset,l,r,z),name+'_'+suffix,material,folder,collide)
    print('TOUGE_ROAD_READY',name)

# This is a terrain/route proposal, not an implemented race system.
for name in ['01_West_Drift','02_East_Drift','03_Valley_Sprint','04_South_Drift_Loop','05_Full_Mountain_Circuit']:
    if not any(a.get_actor_label()==name for a in E.get_all_level_actors()):
        a=E.spawn_actor_from_class(u.Actor,u.Vector())
        a.set_actor_label(name);a.set_folder_path('05_PlannedEvents')
u.EditorAssetLibrary.save_directory(BASE,only_if_is_dirty=True,recursive=True)
assert L.save_current_level()
print('TOUGE_ROADS_COMPLETE')
