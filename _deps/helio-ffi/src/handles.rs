use crate::types::HelioHandle;

/// Convert a handle type that has slot() and generation() into a HelioHandle.
pub fn to_helio_handle<H>(h: H, slot_fn: impl FnOnce(&H) -> u32, gen_fn: impl FnOnce(&H) -> u32) -> HelioHandle {
    HelioHandle {
        slot: slot_fn(&h),
        generation: gen_fn(&h),
    }
}

// ── Per-type conversion helpers ───────────────────────────────────────────────

macro_rules! impl_handle_conv {
    ($helio_type:ty, $from_raw:path) => {
        impl From<$helio_type> for HelioHandle {
            fn from(h: $helio_type) -> Self {
                HelioHandle {
                    slot: h.slot(),
                    generation: h.generation(),
                }
            }
        }
        impl From<HelioHandle> for $helio_type {
            fn from(h: HelioHandle) -> Self {
                $from_raw(h.slot, h.generation)
            }
        }
    };
}

impl_handle_conv!(helio::MeshId, helio::MeshId::from_raw);
impl_handle_conv!(helio::MaterialId, helio::MaterialId::from_raw);
impl_handle_conv!(helio::TextureId, helio::TextureId::from_raw);
impl_handle_conv!(helio::LightId, helio::LightId::from_raw);
impl_handle_conv!(helio::ObjectId, helio::ObjectId::from_raw);
impl_handle_conv!(helio::VirtualObjectId, helio::VirtualObjectId::from_raw);
impl_handle_conv!(helio::WaterVolumeId, helio::WaterVolumeId::from_raw);
impl_handle_conv!(helio::WaterHitboxId, helio::WaterHitboxId::from_raw);
impl_handle_conv!(helio::PostProcessVolumeId, helio::PostProcessVolumeId::from_raw);
impl_handle_conv!(helio::ReflectionCaptureId, helio::ReflectionCaptureId::from_raw);
impl_handle_conv!(helio::VoxelVolumeId, helio::VoxelVolumeId::from_raw);
impl_handle_conv!(helio::DecalId, helio::DecalId::from_raw);
impl_handle_conv!(helio::SectionedInstanceId, helio::SectionedInstanceId::from_raw);
