"""Generate a reproducible 4032 m touge region. No external dependencies.

Import region.r16: 1009x1009, 63 quads/section, 1 section/component,
16x16 components, scale=(400,400,300), location=(-201600,-201600,0).
Design coordinates and elevations are metres; Unreal coordinates are cm.
"""
import math
import json
import struct
from pathlib import Path
from collections import defaultdict

ROUTES = {
    'WestPass': [(-950,-1250,65),(-1150,-1100,78),(-1300,-850,95),(-1030,-720,110),(-900,-540,125),(-1220,-420,142),(-1400,-180,160),(-1130,-100,180),(-950,90,200),(-1260,230,219),(-1330,480,240),(-1050,570,258),(-820,420,272),(-650,620,290),(-900,820,306),(-970,1060,325),(-650,1240,342),(-300,1150,350),(0,1000,345)],
    'EastPass': [(0,1000,345),(300,1200,350),(700,1140,335),(1020,960,318),(1200,710,300),(940,540,280),(700,640,265),(570,430,247),(880,260,228),(1170,350,212),(1320,120,194),(1100,-100,175),(820,-180,157),(1000,-420,139),(1280,-570,120),(1160,-830,103),(850,-1030,87),(550,-1150,73),(120,-1100,65),(-220,-1250,60),(-600,-1350,62),(-950,-1250,65)],
    'ValleyLink': [(-1400,-180,160),(-1170,-330,145),(-920,-250,128),(-700,-420,110),(-430,-300,96),(-200,-470,88),(50,-300,88),(200,-30,98),(450,80,112),(630,-80,135),(820,-180,157)],
    'SouthLink': [(-950,-1250,65),(-820,-1080,71),(-570,-950,78),(-320,-1050,80),(-100,-850,85),(150,-780,92),(300,-560,95),(50,-300,88)]
}

def sample(ctrl, spacing=4):
    pts=[]
    for i in range(len(ctrl)-1):
        p0,p1,p2,p3=ctrl[max(i-1,0)],ctrl[i],ctrl[i+1],ctrl[min(i+2,len(ctrl)-1)]
        n=max(8,int(math.dist(p1,p2)/spacing))
        for k in range(n):
            t=k/n
            pts.append(tuple(.5*((2*p1[j])+(-p0[j]+p2[j])*t+(2*p0[j]-5*p1[j]+4*p2[j]-p3[j])*t*t+(-p0[j]+3*p1[j]-3*p2[j]+p3[j])*t*t*t) for j in range(3)))
    return pts+[ctrl[-1]]

PATHS={key:sample(ctrl) for key,ctrl in ROUTES.items()}
COARSE_PATHS={key:pts[::4] for key,pts in PATHS.items()}
COARSE_HEIGHT={}
BINS=defaultdict(list)
for pts in PATHS.values():
    for p in pts:BINS[(int(p[0]//40),int(p[1]//40))].append(p)

def natural_height(x,y):
    west=430*math.exp(-((x+740)/590)**2-((y-130)/1060)**2)
    east=510*math.exp(-((x-720)/570)**2-((y-470)/1010)**2)
    northern=205*math.exp(-(x/1480)**2-((y-1620)/430)**2)
    southern=65*math.exp(-((x+200)/1400)**2-((y+1700)/450)**2)
    # Correlated multi-scale relief creates spurs and gullies, not random spikes.
    ridges=24*math.sin(x*.007+y*.002)*math.cos(y*.006)
    ridges+=10*math.sin(x*.018+y*.011)*math.cos(y*.014-x*.004)
    ridges+=3.2*math.sin(x*.043+y*.031)+1.1*math.cos(x*.091-y*.073)
    return max(28,45+west+east+northern+southern+ridges)

def corridor_surface(gx,gy):
    key=(gx,gy)
    if key in COARSE_HEIGHT:return COARSE_HEIGHT[key]
    x,y=-2016+gx*32,-2016+gy*32
    total=natural_height(x,y)*.045;weight=.045
    for pts in COARSE_PATHS.values():
        p=min(pts,key=lambda p:(x-p[0])**2+(y-p[1])**2)
        d2=(x-p[0])**2+(y-p[1])**2
        w=math.exp(-d2/(180*180))
        total+=w*(p[2]-.35);weight+=w
    COARSE_HEIGHT[key]=total/weight
    return total/weight

def height(x,y):
    bx,by=int(x//40),int(y//40);best=1e20;z=0
    for dx in (-1,0,1):
        for dy in (-1,0,1):
            for p in BINS.get((bx+dx,by+dy),()):
                d=(x-p[0])**2+(y-p[1])**2
                if d<best:best=d;z=p[2]
    d=math.sqrt(best)
    # Broad, continuously interpolated road-aware hillsides avoid deep slots.
    fx,fy=(x+2016)/32,(y+2016)/32
    ix,iy=int(fx),int(fy);ax,ay=fx-ix,fy-iy
    raw=(corridor_surface(ix,iy)*(1-ax)+corridor_surface(ix+1,iy)*ax)*(1-ay)
    raw+=(corridor_surface(ix,iy+1)*(1-ax)+corridor_surface(ix+1,iy+1)*ax)*ay
    if d>=38:return raw
    a=max(0,min(1,(d-8)/30));a=a*a*(3-2*a)
    return (z-.35)*(1-a)+raw*a

def generate():
    root=Path(__file__).resolve().parents[2]
    out=root/'SourceArt'/'Terrain';out.mkdir(parents=True,exist_ok=True)
    low=1e9;high=-1e9
    with (out/'region.r16').open('wb') as f:
        for j in range(1009):
            y=-2016+j*4
            row=[]
            for i in range(1009):
                h=height(-2016+i*4,y);low=min(low,h);high=max(high,h)
                row.append(max(0,min(65535,round(32768+h*12800/300))))
            f.write(struct.pack('<1009H',*row))
    stats={}
    for name,pts in PATHS.items():
        grades=[abs(b[2]-a[2])/max(.001,math.dist(a[:2],b[:2])) for a,b in zip(pts,pts[1:])]
        stats[name]={'length_m':round(sum(math.dist(a,b) for a,b in zip(pts,pts[1:]))),'max_grade_percent':round(100*max(grades),2)}
    manifest={'resolution':[1009,1009],'size_m':[4032,4032],'height_range_m':[round(low,2),round(high,2)],'landscape_scale':[400,400,300],'landscape_location_cm':[-201600,-201600,0],'roads':stats,'road_centerlines_m':PATHS,'status':'Initial topography and route proposal. Not a finished race or environment.'}
    (out/'region_manifest.json').write_text(json.dumps(manifest,ensure_ascii=False,indent=2),encoding='utf-8')
    print(json.dumps({k:v for k,v in manifest.items() if k!='road_centerlines_m'},indent=2))

if __name__=='__main__':generate()
