#ifndef UTD_ATLAS_IMAGE_GEOM_H
#define UTD_ATLAS_IMAGE_GEOM_H

#define _UTDEXTT_P(_G,_P,_VD) (*_G)[0].position = _P;                           \
							  (*_G)[1].position = _P + Vector2d(0,     _VD[1]); \
							  (*_G)[2].position = _P + Vector2d(_VD[0],_VD[1]); \
							  (*_G)[3].position = _P + Vector2d(_VD[0],0);

#define _UTDQUAD_P(_G,_P,_VD) (*_G)[0].position = _P + Vector2d(_VD[1],_VD[3]); \
							  (*_G)[1].position = _P + Vector2d(_VD[1],_VD[2]); \
							  (*_G)[2].position = _P + Vector2d(_VD[0],_VD[2]); \
							  (*_G)[3].position = _P + Vector2d(_VD[0],_VD[3]);

#define _UTDEXTT(_G,_VD) (*_G)[0].position = Vector2d(0,0);           \
					     (*_G)[1].position = Vector2d(0,     _VD[1]); \
					     (*_G)[2].position = Vector2d(_VD[0],_VD[1]); \
					     (*_G)[3].position = Vector2d(_VD[0],0);

#define _UTDQUAD(_G,_VD) (*_G)[0].position = Vector2d(_VD[1],_VD[3]); \
				         (*_G)[1].position = Vector2d(_VD[1],_VD[2]); \
				         (*_G)[2].position = Vector2d(_VD[0],_VD[2]); \
				         (*_G)[3].position = Vector2d(_VD[0],_VD[3]);

#define _UTDEXTT_INIT(_VD,_EX,_EY)           _VD[0] = _EX;            _VD[1] = _EY;
#define _UTDEXTT_INIT_S(_VD,_EX,_EY,_SX,_SY) _VD[0] = float(_EX)*_SX; _VD[1] = float(_EY)*_SY;

#define _UTDQUAD_INIT(_VD,_EX,_EY) _VD[0] = float(_EX)*0.5f; _VD[1] = -_VD[0]; \
						           _VD[2] = float(_EY)*0.5f; _VD[3] = -_VD[2];
							 
#define _UTDQUAD_INIT_S(_VD,_EX,_EY,_SX,_SY) _VD[0] = float(_EX)*0.5f*_SX; _VD[1] = -_VD[0]; \
							                 _VD[2] = float(_EY)*0.5f*_SY; _VD[3] = -_VD[2];

#define _UTD_DOXFORM(_G,_R,_P) _G->rotate(RotationMatrix(_R)); \
                               _G->translate(_P);
							   
#endif
