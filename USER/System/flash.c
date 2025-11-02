#include "flash.h"
#include <string.h>

// --- 简单校验：magic + 16bit 求和校验 ---
#define MPU_CALIB_MAGIC ((uint32_t)0x4D50554Fu)  // "MPUO"

typedef struct {
  uint32_t magic;      // 固定魔数
  int16_t offsets[6];  // 12字节
  uint16_t checksum;   // 对 magic 与 offsets 的 16 位累加和（不含本字段）
} MpuCalibBlob;

static uint16_t sum16(const void* data, size_t len) {
  const uint8_t* p = (const uint8_t*)data;
  uint32_t sum = 0;
  for (size_t i = 0; i + 1 < len; i += 2) {
    uint16_t hw = (uint16_t)(p[i] | ((uint16_t)p[i + 1] << 8));
    sum += hw;
  }
  if (len & 1) {  // 理论上这里不会触发，因为结构体是偶数长度
    sum += p[len - 1];
  }
  // 折叠到16位
  while (sum >> 16) sum = (sum & 0xFFFFu) + (sum >> 16);
  return (uint16_t)sum;
}

static bool is_blob_valid(const MpuCalibBlob* b) {
  if (b->magic != MPU_CALIB_MAGIC) return FALSE;
  uint16_t calc = sum16(b, sizeof(MpuCalibBlob) - sizeof(uint16_t));
  return (calc == b->checksum) ? TRUE : FALSE;
}

static bool flash_page_erase(uint32_t page_addr) {
  HAL_StatusTypeDef st;
  uint32_t page_error = 0;

  HAL_FLASH_Unlock();
  FLASH_EraseInitTypeDef ei = {0};
  ei.TypeErase = FLASH_TYPEERASE_PAGES;
  ei.PageAddress = page_addr;
  ei.NbPages = 1;

  st = HAL_FLASHEx_Erase(&ei, &page_error);
  HAL_FLASH_Lock();

  return (st == HAL_OK && page_error == 0xFFFFFFFFu) ? TRUE : FALSE;
}

static bool flash_write_halfwords(uint32_t addr, const uint16_t* data, size_t halfword_count) {
  HAL_StatusTypeDef st;
  bool ok = TRUE;

  HAL_FLASH_Unlock();
  for (size_t i = 0; i < halfword_count; ++i) {
    st = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr + (i * 2u), data[i]);
    if (st != HAL_OK) {
      ok = FALSE;
      break;
    }
    // 验证写入
    if (*(volatile uint16_t*)(addr + (i * 2u)) != data[i]) {
      ok = FALSE;
      break;
    }
  }
  HAL_FLASH_Lock();

  return ok;
}

// 外部接口：读取
bool MpuCalib_Load(int16_t outOffsets[6]) {
  if (!outOffsets) return FALSE;

  const MpuCalibBlob* blob = (const MpuCalibBlob*)MPU_CALIB_FLASH_PAGE_ADDR;
  if (!is_blob_valid(blob)) {
    return FALSE;  // 无效数据
  }
  memcpy(outOffsets, blob->offsets, sizeof(int16_t) * 6);
  return TRUE;
}

// 外部接口：保存（会先擦除整页）
bool MpuCalib_Save(const int16_t inOffsets[6]) {
  if (!inOffsets) return FALSE;

  // 如果已有相同数据且有效，可直接返回，避免额外擦写
  int16_t current[6];
  if (MpuCalib_Load(current)) {
    if (memcmp(current, inOffsets, sizeof(current)) == 0) {
      return TRUE;  // 无需重写
    }
  }

  MpuCalibBlob blob = {0};
  blob.magic = MPU_CALIB_MAGIC;
  memcpy(blob.offsets, inOffsets, sizeof(blob.offsets));
  blob.checksum = sum16(&blob, sizeof(MpuCalibBlob) - sizeof(uint16_t));

  // 擦除一整页
  if (!flash_page_erase(MPU_CALIB_FLASH_PAGE_ADDR)) {
    return FALSE;
  }

  // 以半字写入
  size_t halfword_cnt = sizeof(MpuCalibBlob) / 2u;
  return flash_write_halfwords(MPU_CALIB_FLASH_PAGE_ADDR,
                               (const uint16_t*)&blob, halfword_cnt);
}

// 外部接口：清除（擦除页）
bool MpuCalib_Clear(void) {
  return flash_page_erase(MPU_CALIB_FLASH_PAGE_ADDR);
}
