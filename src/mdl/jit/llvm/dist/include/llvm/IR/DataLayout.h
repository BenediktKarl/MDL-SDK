//===- llvm/DataLayout.h - Data size & alignment info -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines layout properties related to datatype size/offset/alignment
// information.  It uses lazy annotations to cache information about how
// structure types are laid out and used.
//
// This structure should be created once, filled in if the defaults are not
// correct and then passed around by const&.  None of the members functions
// require modification to the object.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_DATALAYOUT_H
#define LLVM_IR_DATALAYOUT_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/Alignment.h"
#include "llvm/Support/TypeSize.h"
#include <cassert>
#include <cstdint>
#include <string>

// This needs to be outside of the namespace, to avoid conflict with llvm-c
// decl.
using LLVMTargetDataRef = struct LLVMOpaqueTargetData *;

namespace llvm {

class GlobalVariable;
class LLVMContext;
class Module;
class StructLayout;
class Triple;
class Value;

/// Enum used to categorize the alignment types stored by LayoutAlignElem
enum AlignTypeEnum {
  INVALID_ALIGN = 0,
  INTEGER_ALIGN = 'i',
  VECTOR_ALIGN = 'v',
  FLOAT_ALIGN = 'f',
  AGGREGATE_ALIGN = 'a'
};

// FIXME: Currently the DataLayout string carries a "preferred alignment"
// for types. As the DataLayout is module/global, this should likely be
// sunk down to an FTTI element that is queried rather than a global
// preference.

/// Layout alignment element.
///
/// Stores the alignment data associated with a given alignment type (integer,
/// vector, float) and type bit width.
///
/// \note The unusual order of elements in the structure attempts to reduce
/// padding and make the structure slightly more cache friendly.
struct LayoutAlignElem {
  /// Alignment type from \c AlignTypeEnum
  unsigned AlignType : 8;
  unsigned TypeBitWidth : 24;
  Align ABIAlign;
  Align PrefAlign;

  static LayoutAlignElem get(AlignTypeEnum align_type, Align abi_align,
                             Align pref_align, uint32_t bit_width);

  bool operator==(const LayoutAlignElem &rhs) const;
};

/// Layout pointer alignment element.
///
/// Stores the alignment data associated with a given pointer and address space.
///
/// \note The unusual order of elements in the structure attempts to reduce
/// padding and make the structure slightly more cache friendly.
struct PointerAlignElem {
  Align ABIAlign;
  Align PrefAlign;
  uint32_t TypeByteWidth;
  uint32_t AddressSpace;
  uint32_t IndexWidth;

  /// Initializer
  static PointerAlignElem get(uint32_t AddressSpace, Align ABIAlign,
                              Align PrefAlign, uint32_t TypeByteWidth,
                              uint32_t IndexWidth);

  bool operator==(const PointerAlignElem &rhs) const;
};

/// A parsed version of the target data layout string in and methods for
/// querying it.
///
/// The target data layout string is specified *by the target* - a frontend
/// generating LLVM IR is required to generate the right target data for the
/// target being codegen'd to.
class DataLayout {
public:
  enum class FunctionPtrAlignType {
    /// The function pointer alignment is independent of the function alignment.
    Independent,
    /// The function pointer alignment is a multiple of the function alignment.
    MultipleOfFunctionAlign,
  };
private:
  /// Defaults to false.
  bool BigEndian;

  unsigned AllocaAddrSpace;
  MaybeAlign StackNaturalAlign;
  unsigned ProgramAddrSpace;
  unsigned DefaultGlobalsAddrSpace;

  MaybeAlign FunctionPtrAlign;
  FunctionPtrAlignType TheFunctionPtrAlignType;

  enum ManglingModeT {
    MM_None,
    MM_ELF,
    MM_MachO,
    MM_WinCOFF,
    MM_WinCOFFX86,
    MM_Mips,
    MM_XCOFF
  };
  ManglingModeT ManglingMode;

  SmallVector<unsigned char, 8> LegalIntWidths;

  /// Primitive type alignment data. This is sorted by type and bit
  /// width during construction.
  using AlignmentsTy = SmallVector<LayoutAlignElem, 16>;
  AlignmentsTy Alignments;

  AlignmentsTy::const_iterator
  findAlignmentLowerBound(AlignTypeEnum AlignType, uint32_t BitWidth) const {
    return const_cast<DataLayout *>(this)->findAlignmentLowerBound(AlignType,
                                                                   BitWidth);
  }

  AlignmentsTy::iterator
  findAlignmentLowerBound(AlignTypeEnum AlignType, uint32_t BitWidth);

  /// The string representation used to create this DataLayout
  std::string StringRepresentation;

  using PointersTy = SmallVector<PointerAlignElem, 8>;
  PointersTy Pointers;

  const PointerAlignElem &getPointerAlignElem(uint32_t AddressSpace) const;

  // The StructType -> StructLayout map.
  mutable void *LayoutMap = nullptr;

  /// Pointers in these address spaces are non-integral, and don't have a
  /// well-defined bitwise representation.
  SmallVector<unsigned, 8> NonIntegralAddressSpaces;

  /// Attempts to set the alignment of the given type. Returns an error
  /// description on failure.
  Error setAlignment(AlignTypeEnum align_type, Align abi_align,
                     Align pref_align, uint32_t bit_width);

  /// Attempts to set the alignment of a pointer in the given address space.
  /// Returns an error description on failure.
  Error setPointerAlignment(uint32_t AddrSpace, Align ABIAlign, Align PrefAlign,
                            uint32_t TypeByteWidth, uint32_t IndexWidth);

  /// Internal helper to get alignment for integer of given bitwidth.
  Align getIntegerAlignment(uint32_t BitWidth, bool abi_or_pref) const;

  /// Internal helper method that returns requested alignment for type.
  Align getAlignment(Type *Ty, bool abi_or_pref) const;

  /// Attempts to parse a target data specification string and reports an error
  /// if the string is malformed.
  Error parseSpecifier(StringRef Desc);

  // Free all internal data structures.
  void clear();

public:
  /// Constructs a DataLayout from a specification string. See reset().
  explicit DataLayout(StringRef LayoutDescription) {
    reset(LayoutDescription);
  }

  /// Initialize target data from properties stored in the module.
  explicit DataLayout(const Module *M);

  DataLayout(const DataLayout &DL) { *this = DL; }

  ~DataLayout(); // Not virtual, do not subclass this class

  DataLayout &operator=(const DataLayout &DL) {
    clear();
    StringRepresentation = DL.StringRepresentation;
    BigEndian = DL.isBigEndian();
    AllocaAddrSpace = DL.AllocaAddrSpace;
    StackNaturalAlign = DL.StackNaturalAlign;
    FunctionPtrAlign = DL.FunctionPtrAlign;
    TheFunctionPtrAlignType = DL.TheFunctionPtrAlignType;
    ProgramAddrSpace = DL.ProgramAddrSpace;
    DefaultGlobalsAddrSpace = DL.DefaultGlobalsAddrSpace;
    ManglingMode = DL.ManglingMode;
    LegalIntWidths = DL.LegalIntWidths;
    Alignments = DL.Alignments;
    Pointers = DL.Pointers;
    NonIntegralAddressSpaces = DL.NonIntegralAddressSpaces;
    return *this;
  }

  bool operator==(const DataLayout &Other) const;
  bool operator!=(const DataLayout &Other) const { return !(*this == Other); }

  void init(const Module *M);

  /// Parse a data layout string (with fallback to default values).
  void reset(StringRef LayoutDescription);

  /// Parse a data layout string and return the layout. Return an error
  /// description on failure.
  static Expected<DataLayout> parse(StringRef LayoutDescription);

  /// Layout endianness...
  bool isLittleEndian() const { return !BigEndian; }
  bool isBigEndian() const { return BigEndian; }

  /// Returns the string representation of the DataLayout.
  ///
  /// This representation is in the same format accepted by the string
  /// constructor above. This should not be used to compare two DataLayout as
  /// different string can represent the same layout.
  const std::string &getStringRepresentation() const {
    return StringRepresentation;
  }

  /// Test if the DataLayout was constructed from an empty string.
  bool isDefault() const { return StringRepresentation.empty(); }

  /// Returns true if the specified type is known to be a native integer
  /// type supported by the CPU.
  ///
  /// For example, i64 is not native on most 32-bit CPUs and i37 is not native
  /// on any known one. This returns false if the integer width is not legal.
  ///
  /// The width is specified in bits.
  bool isLegalInteger(uint64_t Width) const {
    for (unsigned LegalIntWidth : LegalIntWidths)
      if (LegalIntWidth == Width)
        return true;
    return false;
  }

  bool isIllegalInteger(uint64_t Width) const { return !isLegalInteger(Width); }

  /// Returns true if the given alignment exceeds the natural stack alignment.
  bool exceedsNaturalStackAlignment(Align Alignment) const {
    return StackNaturalAlign && (Alignment > *StackNaturalAlign);
  }

  Align getStackAlignment() const {
    assert(StackNaturalAlign && "StackNaturalAlign must be defined");
    return *StackNaturalAlign;
  }

  unsigned getAllocaAddrSpace() const { return AllocaAddrSpace; }

  /// Returns the alignment of function pointers, which may or may not be
  /// related to the alignment of functions.
  /// \see getFunctionPtrAlignType
  MaybeAlign getFunctionPtrAlign() const { return FunctionPtrAlign; }

  /// Return the type of function pointer alignment.
  /// \see getFunctionPtrAlign
  FunctionPtrAlignType getFunctionPtrAlignType() const {
    return TheFunctionPtrAlignType;
  }

  unsigned getProgramAddressSpace() const { return ProgramAddrSpace; }
  unsigned getDefaultGlobalsAddressSpace() const {
    return DefaultGlobalsAddrSpace;
  }

  bool hasMicrosoftFastStdCallMangling() const {
    return ManglingMode == MM_WinCOFFX86;
  }

  /// Returns true if symbols with leading question marks should not receive IR
  /// mangling. True for Windows mangling modes.
  bool doNotMangleLeadingQuestionMark() const {
    return ManglingMode == MM_WinCOFF || ManglingMode == MM_WinCOFFX86;
  }

  bool hasLinkerPrivateGlobalPrefix() const { return ManglingMode == MM_MachO; }

  StringRef getLinkerPrivateGlobalPrefix() const {
    if (ManglingMode == MM_MachO)
      return "l";
    return "";
  }

  char getGlobalPrefix() const {
    switch (ManglingMode) {
    case MM_None:
    case MM_ELF:
    case MM_Mips:
    case MM_WinCOFF:
    case MM_XCOFF:
      return '\0';
    case MM_MachO:
    case MM_WinCOFFX86:
      return '_';
    }
    llvm_unreachable("invalid mangling mode");
  }

  StringRef getPrivateGlobalPrefix() const {
    switch (ManglingMode) {
    case MM_None:
      return "";
    case MM_ELF:
    case MM_WinCOFF:
      return ".L";
    case MM_Mips:
      return "$";
    case MM_MachO:
    case MM_WinCOFFX86:
      return "L";
    case MM_XCOFF:
      return "L..";
    }
    llvm_unreachable("invalid mangling mode");
  }

  static const char *getManglingComponent(const Triple &T);

  /// Returns true if the specified type fits in a native integer type
  /// supported by the CPU.
  ///
  /// For example, if the CPU only supports i32 as a native integer type, then
  /// i27 fits in a legal integer type but i45 does not.
  bool fitsInLegalInteger(unsigned Width) const {
    for (unsigned LegalIntWidth : LegalIntWidths)
      if (Width <= LegalIntWidth)
        return true;
    return false;
  }

  /// Layout pointer alignment
  Align getPointerABIAlignment(unsigned AS) const;

  /// Return target's alignment for stack-based pointers
  /// FIXME: The defaults need to be removed once all of
  /// the backends/clients are updated.
  Align getPointerPrefAlignment(unsigned AS = 0) const;

  /// Layout pointer size
  /// FIXME: The defaults need to be removed once all of
  /// the backends/clients are updated.
  unsigned getPointerSize(unsigned AS = 0) const;

  /// Returns the maximum pointer size over all address spaces.
  unsigned getMaxPointerSize() const;

  // Index size used for address calculation.
  unsigned getIndexSize(unsigned AS) const;

  /// Return the address spaces containing non-integral pointers.  Pointers in
  /// this address space don't have a well-defined bitwise representation.
  ArrayRef<unsigned> getNonIntegralAddressSpaces() const {
    return NonIntegralAddressSpaces;
  }

  bool isNonIntegralAddressSpace(unsigned AddrSpace) const {
    ArrayRef<unsigned> NonIntegralSpaces = getNonIntegralAddressSpaces();
    return is_contained(NonIntegralSpaces, AddrSpace);
  }

  bool isNonIntegralPointerType(PointerType *PT) const {
    return isNonIntegralAddressSpace(PT->getAddressSpace());
  }

  bool isNonIntegralPointerType(Type *Ty) const {
    auto *PTy = dyn_cast<PointerType>(Ty);
    return PTy && isNonIntegralPointerType(PTy);
  }

  /// Layout pointer size, in bits
  /// FIXME: The defaults need to be removed once all of
  /// the backends/clients are updated.
  unsigned getPointerSizeInBits(unsigned AS = 0) const {
    return getPointerSize(AS) * 8;
  }

  /// Returns the maximum pointer size over all address spaces.
  unsigned getMaxPointerSizeInBits() const {
    return getMaxPointerSize() * 8;
  }

  /// Size in bits of index used for address calculation in getelementptr.
  unsigned getIndexSizeInBits(unsigned AS) const {
    return getIndexSize(AS) * 8;
  }

  /// Layout pointer size, in bits, based on the type.  If this function is
  /// called with a pointer type, then the type size of the pointer is returned.
  /// If this function is called with a vector of pointers, then the type size
  /// of the pointer is returned.  This should only be called with a pointer or
  /// vector of pointers.
  unsigned getPointerTypeSizeInBits(Type *) const;

  /// Layout size of the index used in GEP calculation.
  /// The function should be called with pointer or vector of pointers type.
  unsigned getIndexTypeSizeInBits(Type *Ty) const;

  unsigned getPointerTypeSize(Type *Ty) const {
    return getPointerTypeSizeInBits(Ty) / 8;
  }

  /// Size examples:
  ///
  /// Type        SizeInBits  StoreSizeInBits  AllocSizeInBits[*]
  /// ----        ----------  ---------------  ---------------
  ///  i1            1           8                8
  ///  i8            8           8                8
  ///  i19          19          24               32
  ///  i32          32          32               32
  ///  i100        100         104              128
  ///  i128        128         128              128
  ///  Float        32          32               32
  ///  Double       64          64               64
  ///  X86_FP80     80          80               96
  ///
  /// [*] The alloc size depends on the alignment, and thus on the target.
  ///     These values are for x86-32 linux.

  /// Returns the number of bits necessary to hold the specified type.
  ///
  /// If Ty is a scalable vector type, the scalable property will be set and
  /// the runtime size will be a positive integer multiple of the base size.
  ///
  /// For example, returns 36 for i36 and 80 for x86_fp80. The type passed must
  /// have a size (Type::isSized() must return true).
  TypeSize getTypeSizeInBits(Type *Ty) const;

  /// Returns the maximum number of bytes that may be overwritten by
  /// storing the specified type.
  ///
  /// If Ty is a scalable vector type, the scalable property will be set and
  /// the runtime size will be a positive integer multiple of the base size.
  ///
  /// For example, returns 5 for i36 and 10 for x86_fp80.
  TypeSize getTypeStoreSize(Type *Ty) const {
    TypeSize BaseSize = getTypeSizeInBits(Ty);
    return { (BaseSize.getKnownMinSize() + 7) / 8, BaseSize.isScalable() };
  }

  /// Returns the maximum number of bits that may be overwritten by
  /// storing the specified type; always a multiple of 8.
  ///
  /// If Ty is a scalable vector type, the scalable property will be set and
  /// the runtime size will be a positive integer multiple of the base size.
  ///
  /// For example, returns 40 for i36 and 80 for x86_fp80.
  TypeSize getTypeStoreSizeInBits(Type *Ty) const {
    return 8 * getTypeStoreSize(Ty);
  }

  /// Returns true if no extra padding bits are needed when storing the
  /// specified type.
  ///
  /// For example, returns false for i19 that has a 24-bit store size.
  bool typeSizeEqualsStoreSize(Type *Ty) const {
    return getTypeSizeInBits(Ty) == getTypeStoreSizeInBits(Ty);
  }

  /// Returns the offset in bytes between successive objects of the
  /// specified type, including alignment padding.
  ///
  /// If Ty is a scalable vector type, the scalable property will be set and
  /// the runtime size will be a positive integer multiple of the base size.
  ///
  /// This is the amount that alloca reserves for this type. For example,
  /// returns 12 or 16 for x86_fp80, depending on alignment.
  TypeSize getTypeAllocSize(Type *Ty) const {
    // Round up to the next alignment boundary.
    return alignTo(getTypeStoreSize(Ty), getABITypeAlignment(Ty));
  }

  /// Returns the offset in bits between successive objects of the
  /// specified type, including alignment padding; always a multiple of 8.
  ///
  /// If Ty is a scalable vector type, the scalable property will be set and
  /// the runtime size will be a positive integer multiple of the base size.
  ///
  /// This is the amount that alloca reserves for this type. For example,
  /// returns 96 or 128 for x86_fp80, depending on alignment.
  TypeSize getTypeAllocSizeInBits(Type *Ty) const {
    return 8 * getTypeAllocSize(Ty);
  }

  /// Returns the minimum ABI-required alignment for the specified type.
  /// FIXME: Deprecate this function once migration to Align is over.
  unsigned getABITypeAlignment(Type *Ty) const;

  /// Returns the minimum ABI-required alignment for the specified type.
  Align getABITypeAlign(Type *Ty) const;

  /// Helper function to return `Alignment` if it's set or the result of
  /// `getABITypeAlignment(Ty)`, in any case the result is a valid alignment.
  inline Align getValueOrABITypeAlignment(MaybeAlign Alignment,
                                          Type *Ty) const {
    return Alignment ? *Alignment : getABITypeAlign(Ty);
  }

  /// Returns the minimum ABI-required alignment for an integer type of
  /// the specified bitwidth.
  Align getABIIntegerTypeAlignment(unsigned BitWidth) const {
    return getIntegerAlignment(BitWidth, /* abi_or_pref */ true);
  }

  /// Returns the preferred stack/global alignment for the specified
  /// type.
  ///
  /// This is always at least as good as the ABI alignment.
  /// FIXME: Deprecate this function once migration to Align is over.
  unsigned getPrefTypeAlignment(Type *Ty) const;

  /// Returns the preferred stack/global alignment for the specified
  /// type.
  ///
  /// This is always at least as good as the ABI alignment.
  Align getPrefTypeAlign(Type *Ty) const;

  /// Returns an integer type with size at least as big as that of a
  /// pointer in the given address space.
  IntegerType *getIntPtrType(LLVMContext &C, unsigned AddressSpace = 0) const;

  /// Returns an integer (vector of integer) type with size at least as
  /// big as that of a pointer of the given pointer (vector of pointer) type.
  Type *getIntPtrType(Type *) const;

  /// Returns the smallest integer type with size at least as big as
  /// Width bits.
  Type *getSmallestLegalIntType(LLVMContext &C, unsigned Width = 0) const;

  /// Returns the largest legal integer type, or null if none are set.
  Type *getLargestLegalIntType(LLVMContext &C) const {
    unsigned LargestSize = getLargestLegalIntTypeSizeInBits();
    return (LargestSize == 0) ? nullptr : Type::getIntNTy(C, LargestSize);
  }

  /// Returns the size of largest legal integer type size, or 0 if none
  /// are set.
  unsigned getLargestLegalIntTypeSizeInBits() const;

  /// Returns the type of a GEP index.
  /// If it was not specified explicitly, it will be the integer type of the
  /// pointer width - IntPtrType.
  Type *getIndexType(Type *PtrTy) const;

  /// Returns the offset from the beginning of the type for the specified
  /// indices.
  ///
  /// Note that this takes the element type, not the pointer type.
  /// This is used to implement getelementptr.
  int64_t getIndexedOffsetInType(Type *ElemTy, ArrayRef<Value *> Indices) const;

  /// Returns a StructLayout object, indicating the alignment of the
  /// struct, its size, and the offsets of its fields.
  ///
  /// Note that this information is lazily cached.
  const StructLayout *getStructLayout(StructType *Ty) const;

  /// Returns the preferred alignment of the specified global.
  ///
  /// This includes an explicitly requested alignment (if the global has one).
  Align getPreferredAlign(const GlobalVariable *GV) const;

  /// Returns the preferred alignment of the specified global.
  ///
  /// This includes an explicitly requested alignment (if the global has one).
  LLVM_ATTRIBUTE_DEPRECATED(
      inline unsigned getPreferredAlignment(const GlobalVariable *GV) const,
      "Use getPreferredAlign instead") {
    return unsigned(getPreferredAlign(GV).value());
  }

  /// Returns the preferred alignment of the specified global, returned
  /// in log form.
  ///
  /// This includes an explicitly requested alignment (if the global has one).
  LLVM_ATTRIBUTE_DEPRECATED(
      inline unsigned getPreferredAlignmentLog(const GlobalVariable *GV) const,
      "Inline where needed") {
    return Log2(getPreferredAlign(GV));
  }
};

inline DataLayout *unwrap(LLVMTargetDataRef P) {
  return reinterpret_cast<DataLayout *>(P);
}

inline LLVMTargetDataRef wrap(const DataLayout *P) {
  return reinterpret_cast<LLVMTargetDataRef>(const_cast<DataLayout *>(P));
}

/// Used to lazily calculate structure layout information for a target machine,
/// based on the DataLayout structure.
class StructLayout {
  uint64_t StructSize;
  Align StructAlignment;
  unsigned IsPadded : 1;
  unsigned NumElements : 31;
  uint64_t MemberOffsets[1]; // variable sized array!

public:
  uint64_t getSizeInBytes() const { return StructSize; }

  uint64_t getSizeInBits() const { return 8 * StructSize; }

  Align getAlignment() const { return StructAlignment; }

  /// Returns whether the struct has padding or not between its fields.
  /// NB: Padding in nested element is not taken into account.
  bool hasPadding() const { return IsPadded; }

  /// Given a valid byte offset into the structure, returns the structure
  /// index that contains it.
  unsigned getElementContainingOffset(uint64_t Offset) const;

  uint64_t getElementOffset(unsigned Idx) const {
    assert(Idx < NumElements && "Invalid element idx!");
    return MemberOffsets[Idx];
  }

  uint64_t getElementOffsetInBits(unsigned Idx) const {
    return getElementOffset(Idx) * 8;
  }

private:
  friend class DataLayout; // Only DataLayout can create this class

  StructLayout(StructType *ST, const DataLayout &DL);
};

// The implementation of this method is provided inline as it is particularly
// well suited to constant folding when called on a specific Type subclass.
inline TypeSize DataLayout::getTypeSizeInBits(Type *Ty) const {
  assert(Ty->isSized() && "Cannot getTypeInfo() on a type that is unsized!");
  switch (Ty->getTypeID()) {
  case Type::LabelTyID:
    return TypeSize::Fixed(getPointerSizeInBits(0));
  case Type::PointerTyID:
    return TypeSize::Fixed(getPointerSizeInBits(Ty->getPointerAddressSpace()));
  case Type::ArrayTyID: {
    ArrayType *ATy = cast<ArrayType>(Ty);
    return ATy->getNumElements() *
           getTypeAllocSizeInBits(ATy->getElementType());
  }
  case Type::StructTyID:
    // Get the layout annotation... which is lazily created on demand.
    return TypeSize::Fixed(
                        getStructLayout(cast<StructType>(Ty))->getSizeInBits());
  case Type::IntegerTyID:
    return TypeSize::Fixed(Ty->getIntegerBitWidth());
  case Type::HalfTyID:
  case Type::BFloatTyID:
    return TypeSize::Fixed(16);
  case Type::FloatTyID:
    return TypeSize::Fixed(32);
  case Type::DoubleTyID:
  case Type::X86_MMXTyID:
    return TypeSize::Fixed(64);
  case Type::PPC_FP128TyID:
  case Type::FP128TyID:
    return TypeSize::Fixed(128);
  case Type::X86_AMXTyID:
    return TypeSize::Fixed(8192);
  // In memory objects this is always aligned to a higher boundary, but
  // only 80 bits contain information.
  case Type::X86_FP80TyID:
    return TypeSize::Fixed(80);
  case Type::FixedVectorTyID:
  case Type::ScalableVectorTyID: {
    VectorType *VTy = cast<VectorType>(Ty);
    auto EltCnt = VTy->getElementCount();
    uint64_t MinBits = EltCnt.getKnownMinValue() *
                       getTypeSizeInBits(VTy->getElementType()).getFixedSize();
    return TypeSize(MinBits, EltCnt.isScalable());
  }
  default:
    llvm_unreachable("DataLayout::getTypeSizeInBits(): Unsupported type");
  }
}

} // end namespace llvm

#endif // LLVM_IR_DATALAYOUT_H
