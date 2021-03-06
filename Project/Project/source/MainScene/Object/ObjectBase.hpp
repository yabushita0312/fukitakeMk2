#pragma once
#include"Header.hpp"

namespace FPS_n2 {
	namespace Sceneclass {
		class ObjectBaseClass {
		protected:
			MV1											m_obj;
			MV1											m_col;
			moves										m_move;
			MATRIX_ref									m_PrevMat;//物理更新のため
			const MV1*									m_MapCol{ nullptr };
			std::vector<std::pair<int, MATRIX_ref>>		Frames;
			std::vector<std::pair<int, float>>			Shapes;
			ObjType										m_objType{ ObjType::Human };
			std::string									m_FilePath;
			std::string									m_ObjFileName;
			std::string									m_ColFileName;
			bool										m_IsActive{ true };
			bool										m_IsResetPhysics{ true };
			bool										m_IsFirstLoop{ true };
			bool										m_IsDraw{ true };
			float										m_DistanceToCam{ 0.f };
			bool										m_IsBaseModel{ false };
		public:
			void			SetActive(bool value) noexcept { this->m_IsActive = value; }
			void			SetMapCol(const MV1* MapCol) noexcept { this->m_MapCol = MapCol; }
			void			SetResetP(bool value) { this->m_IsResetPhysics = value; }
			const auto		GetMatrix(void) const noexcept { return this->m_obj.GetMatrix(); }
			const auto		GetIsBaseModel(const char* filepath, const char* objfilename, const char* colfilename) const noexcept {
				return (
					this->m_IsBaseModel &&
					(this->m_FilePath == filepath) &&
					(this->m_ObjFileName == objfilename) &&
					(this->m_ColFileName == colfilename));
			}
			const auto&		GetobjType(void) const noexcept { return this->m_objType; }
			const auto&		GetCol(void) const noexcept { return this->m_col; }
			//
			void			SetAnimOnce(int ID, float speed) {
				this->m_obj.get_anime(ID).time += 30.f / FPS * speed;
				if (this->m_obj.get_anime(ID).TimeEnd()) { this->m_obj.get_anime(ID).GoEnd(); }
			}
			void			SetAnimLoop(int ID, float speed) {
				this->m_obj.get_anime(ID).time += 30.f / FPS * speed;
				if (this->m_obj.get_anime(ID).TimeEnd()) { this->m_obj.get_anime(ID).GoStart(); }
			}
			void			SetMove(const MATRIX_ref& mat, const VECTOR_ref& pos) noexcept {
				this->m_move.mat = mat;
				this->m_move.pos = pos;
				UpdateMove();
			}
			void			UpdateMove(void) noexcept {
				this->m_PrevMat = this->m_obj.GetMatrix();
				this->m_obj.SetMatrix(this->m_move.MatIn());
				if (this->m_col.IsActive()) {
					this->m_col.SetMatrix(this->m_move.MatIn());
					this->m_col.RefreshCollInfo();
				}
			}
		public:
			void			LoadModel(const char* filepath, int PhysicsType, const char* objfilename = "model", const char* colfilename = "col") noexcept {
				this->m_FilePath = filepath;
				this->m_ObjFileName = objfilename;
				this->m_ColFileName = colfilename;
				FILEINFO FileInfo;
				//model
				{
					std::string Path = this->m_FilePath;
					Path += this->m_ObjFileName;
					if (FileRead_findFirst((Path + ".mv1").c_str(), &FileInfo) != (DWORD_PTR)-1) {
						//MV1::Load(Path + ".pmx", &this->m_obj, PhysicsType);
						MV1::Load((Path + ".mv1").c_str(), &this->m_obj, PhysicsType);
					}
					else {
						MV1::Load(Path + ".pmx", &this->m_obj, PhysicsType);
						MV1SetLoadModelUsePhysicsMode(PhysicsType);
						MV1SaveModelToMV1File(this->m_obj.get(), (Path + ".mv1").c_str());
						MV1SetLoadModelUsePhysicsMode(DX_LOADMODEL_PHYSICS_LOADCALC);
					}
					MV1::SetAnime(&this->m_obj, this->m_obj);
				}
				//col
				{
					std::string Path = this->m_FilePath;
					Path += this->m_ColFileName;
					if (FileRead_findFirst((Path + ".mv1").c_str(), &FileInfo) != (DWORD_PTR)-1) {
						MV1::Load(Path + ".pmx", &this->m_col, DX_LOADMODEL_PHYSICS_REALTIME);
						//MV1::Load((Path + ".mv1").c_str(), &this->m_col, DX_LOADMODEL_PHYSICS_REALTIME);
					}
					else {
						if (FileRead_findFirst((Path + ".pmx").c_str(), &FileInfo) != (DWORD_PTR)-1) {
							MV1::Load(Path + ".pmx", &this->m_col, DX_LOADMODEL_PHYSICS_REALTIME);
							MV1SetLoadModelUsePhysicsMode(DX_LOADMODEL_PHYSICS_REALTIME);
							MV1SaveModelToMV1File(this->m_col.get(), (Path + ".mv1").c_str());
							MV1SetLoadModelUsePhysicsMode(DX_LOADMODEL_PHYSICS_LOADCALC);
						}
						else {
						}
					}

					this->m_col.SetupCollInfo(1, 1, 1);
				}
				this->m_IsBaseModel = true;
			}
			void			CopyModel(std::shared_ptr<ObjectBaseClass>& pBase) noexcept {
				this->m_FilePath = pBase->m_FilePath;
				this->m_ObjFileName = pBase->m_ObjFileName;
				this->m_ColFileName = pBase->m_ColFileName;
				this->m_obj = pBase->m_obj.Duplicate();
				MV1::SetAnime(&this->m_obj, pBase->m_obj);
				//col
				if (pBase->m_col.IsActive()) {
					this->m_col = pBase->m_col.Duplicate();
					this->m_col.SetupCollInfo(1, 1, 1);
				}
				this->m_IsBaseModel = false;
			}
			//
			virtual void	Init(void) noexcept {
				this->m_IsActive = true;
				this->m_IsResetPhysics = true;
				this->m_IsFirstLoop = true;
				this->m_IsDraw = false;
			}
			//
			void			SetFrameNum(void) noexcept {
				int i = 0;
				bool isEnd = false;
				auto fNum = this->m_obj.frame_num();
				for (int f = 0; f < fNum; f++) {
					std::string FName = this->m_obj.frame_name(f);
					bool compare = false;
					switch (this->m_objType) {
					case ObjType::Human://human
						compare = (FName == CharaFrameName[i]);
						break;
					case ObjType::Gun://human
						compare = (FName == GunFrameName[i]);
						break;
					default:
						break;
					}

					if (compare) {
						this->Frames.resize(this->Frames.size() + 1);
						this->Frames.back().first = f;
						this->Frames.back().second = MATRIX_ref::Mtrans(this->m_obj.GetFrameLocalMatrix(this->Frames.back().first).pos());
						i++;
						f = 0;
					}
					switch (this->m_objType) {
					case ObjType::Human://human
						if (i == (int)CharaFrame::Max) { isEnd = true; }
						break;
					case ObjType::Gun://human
						if (i == (int)GunFrame::Max) { isEnd = true; }
						break;
					default:
						isEnd = true;
						break;
					}
					if (f == fNum - 1) {
						if (!isEnd) {
							this->Frames.resize(this->Frames.size() + 1);
							this->Frames.back().first = -1;
							i++;
							f = 0;
						}
					}
					if (isEnd) {
						break;
					}
				}
				switch (this->m_objType) {
				case ObjType::Human://human
					Shapes.resize((int)CharaShape::Max);
					for (int j = 1; j < (int)CharaShape::Max; j++) {
						auto s = MV1SearchShape(this->m_obj.get(), CharaShapeName[j]);
						if (s >= 0) {
							Shapes[j].first = s;
							Shapes[j].second = 0.f;
						}
					}
					break;
				default:
					break;
				}
			}
			//
			virtual void	Execute(void) noexcept { }
			void			ExecuteCommon(void) noexcept {
				if (this->m_IsFirstLoop) {
					this->m_PrevMat = this->m_obj.GetMatrix();
				}
				//シェイプ更新
				switch (this->m_objType) {
				case ObjType::Human://human
					for (int j = 1; j < (int)CharaShape::Max; j++) {
						MV1SetShapeRate(this->m_obj.get(), Shapes[j].first, (1.f - Shapes[0].second)*Shapes[j].second);
					}
					break;
				default:
					break;
				}
				//物理更新
				{
					if (this->m_IsResetPhysics) {
						this->m_IsResetPhysics = false;
						this->m_obj.PhysicsResetState();
					}
					else {
						auto NowMat = this->m_obj.GetMatrix();
						int Max = 2;
						for (int i = 1; i <= Max; i++) {
							this->m_obj.SetMatrix(
								
								Leap_Matrix(this->m_PrevMat.GetRot(), NowMat.GetRot(), (float)i / (float)Max)
								* MATRIX_ref::Mtrans(Leap(this->m_PrevMat.pos(), NowMat.pos(), (float)i / (float)Max)));
							this->m_obj.PhysicsCalculation(1000.0f / FPS * 60.f);
						}
					}
				}
				//最初のループ終わり
				this->m_IsFirstLoop = false;
			}
			//
			virtual void	Depth_Draw(void) noexcept { }
			virtual void	DrawShadow(void) noexcept {
				if (this->m_IsActive) {
					this->m_obj.DrawModel();
				}
			}
			void			CheckDraw(void) noexcept {
				this->m_IsDraw = false;
				this->m_DistanceToCam = (m_obj.GetMatrix().pos() - GetCameraPosition()).size();
				if (CheckCameraViewClip_Box(
					(m_obj.GetMatrix().pos() + VECTOR_ref::vget(-20, 0, -20)).get(),
					(m_obj.GetMatrix().pos() + VECTOR_ref::vget(20, 20, 20)).get()) == FALSE
					) {
					this->m_IsDraw |= true;
				}
			}
			virtual void	Draw(void) noexcept {
				if (this->m_IsActive && this->m_IsDraw) {
					if (CheckCameraViewClip_Box(
						(m_obj.GetMatrix().pos() + VECTOR_ref::vget(-20, 0, -20)).get(),
						(m_obj.GetMatrix().pos() + VECTOR_ref::vget(20, 20, 20)).get()) == FALSE
						) {
						this->m_obj.DrawModel();
					}
				}
			}
			//
			virtual void	Dispose(void) noexcept {
				this->m_obj.Dispose();
			}
		};
	};
};
