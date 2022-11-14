/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTypeface_DEFINED
#define SkTypeface_DEFINED

#include "SkFontArguments.h"
#include "SkFontParameters.h"
#include "SkFontStyle.h"
#include "SkFontTypes.h"
// #include "include/core/SkRect.h"
// #include "include/core/SkString.h"
// #include "include/private/SkOnce.h"
// #include "include/private/SkWeakRefCnt.h"

class SkData;
class SkDescriptor;
class SkFontData;
class SkFontDescriptor;
class SkScalerContext;
class SkStream;
class SkStreamAsset;
class SkWStream;
struct SkAdvancedTypefaceMetrics;
struct SkScalerContextEffects;
struct SkScalerContextRec;

typedef uint32_t SkFontID;
/** Machine endian. */
typedef uint32_t SkFontTableTag;

class SK_API SkTypeface : public SkWeakRefCnt {
public:
    enum class SerializeBehavior {
        kDoIncludeData,
        kDontIncludeData,
        kIncludeDataIfLocal,
    };
    struct LocalizedString {
        SkString fString;
        SkString fLanguage;
    };
    class LocalizedStrings {
    public:
        LocalizedStrings() = default;
        virtual ~LocalizedStrings() { }
        virtual bool next(LocalizedString* localizedString) = 0;
        void unref() { delete this; }
    private:
        LocalizedStrings(const LocalizedStrings&) = delete;
        LocalizedStrings& operator=(const LocalizedStrings&) = delete;
    };

    /** Returns the typeface's intrinsic style attributes. */
    SkFontStyle fontStyle() const {
        return fStyle;
    }
    bool isBold() const { return fStyle.weight() >= SkFontStyle::kSemiBold_Weight; }
    bool isItalic() const { return fStyle.slant() != SkFontStyle::kUpright_Slant; }
    bool isFixedPitch() const { return fIsFixedPitch; }
    int getVariationDesignPosition(SkFontArguments::VariationPosition::Coordinate coordinates[], int coordinateCount) const;
    int getVariationDesignParameters(SkFontParameters::Variation::Axis parameters[], int parameterCount) const;
    SkFontID uniqueID() const { return fUniqueID; }

    static SkFontID UniqueID(const SkTypeface* face);
    static bool Equal(const SkTypeface* facea, const SkTypeface* faceb);
    static SkTypeface MakeDefault();
    static SkTypeface MakeFromName(const char familyName[], SkFontStyle fontStyle);
    static SkTypeface MakeFromFile(const char path[], int index = 0);
    static SkTypeface MakeFromStream(std::unique_ptr<SkStreamAsset> stream, int index = 0);
    static SkTypeface MakeFromData(sk_sp<SkData>, int index = 0);
    static SkTypeface MakeDeserialize(SkStream*);

    SkTypeface makeClone(const SkFontArguments&) const;

    void serialize(SkWStream*, SerializeBehavior = SerializeBehavior::kIncludeDataIfLocal) const;
    sk_sp<SkData> serialize(SerializeBehavior = SerializeBehavior::kIncludeDataIfLocal) const;
    
    void unicharsToGlyphs(const SkUnichar uni[], int count, SkGlyphID glyphs[]) const;
    int textToGlyphs(const void* text, size_t byteLength, SkTextEncoding encoding, SkGlyphID glyphs[], int maxGlyphCount) const;
    SkGlyphID unicharToGlyph(SkUnichar unichar) const;
    int countGlyphs() const;
    int countTables() const;
    int getTableTags(SkFontTableTag tags[]) const;
    size_t getTableSize(SkFontTableTag) const;
    size_t getTableData(SkFontTableTag tag, size_t offset, size_t length, void* data) const;
    sk_sp<SkData> copyTableData(SkFontTableTag tag) const;
    int getUnitsPerEm() const;
    bool getKerningPairAdjustments(const SkGlyphID glyphs[], int count,
                                   int32_t adjustments[]) const;

    LocalizedStrings* createFamilyNameIterator() const;
    void getFamilyName(SkString* name) const;
    bool getPostScriptName(SkString* name) const;
    std::unique_ptr<SkStreamAsset> openStream(int* ttcIndex) const;
    std::unique_ptr<SkScalerContext> createScalerContext(const SkScalerContextEffects&, const SkDescriptor*) const;
    SkRect getBounds() const;

    // PRIVATE / EXPERIMENTAL -- do not call
    void filterRec(SkScalerContextRec* rec) const {
        this->onFilterRec(rec);
    }
    // PRIVATE / EXPERIMENTAL -- do not call
    void getFontDescriptor(SkFontDescriptor* desc, bool* isLocal) const {
        this->onGetFontDescriptor(desc, isLocal);
    }
    // PRIVATE / EXPERIMENTAL -- do not call
    void* internal_private_getCTFontRef() const {
        return this->onGetCTFontRef();
    }

protected:
    explicit SkTypeface(const SkFontStyle& style, bool isFixedPitch = false);
    ~SkTypeface() override;
    void setIsFixedPitch(bool isFixedPitch) { fIsFixedPitch = isFixedPitch; }
    void setFontStyle(SkFontStyle style) { fStyle = style; }
    virtual sk_sp<SkTypeface> onMakeClone(const SkFontArguments&) const = 0;
    virtual std::unique_ptr<SkScalerContext> onCreateScalerContext(const SkScalerContextEffects&, const SkDescriptor*) const = 0;
    virtual void onFilterRec(SkScalerContextRec*) const = 0;
    friend class SkScalerContext;  // onFilterRec
    virtual std::unique_ptr<SkAdvancedTypefaceMetrics> onGetAdvancedMetrics() const = 0;
    virtual void getPostScriptGlyphNames(SkString*) const = 0;
    virtual void getGlyphToUnicodeMap(SkUnichar* dstArray) const = 0;
    virtual std::unique_ptr<SkStreamAsset> onOpenStream(int* ttcIndex) const = 0;
    virtual int onGetVariationDesignPosition(SkFontArguments::VariationPosition::Coordinate coordinates[], int coordinateCount) const = 0;
    virtual int onGetVariationDesignParameters(SkFontParameters::Variation::Axis parameters[], int parameterCount) const = 0;
    virtual void onGetFontDescriptor(SkFontDescriptor*, bool* isLocal) const = 0;
    virtual void onCharsToGlyphs(const SkUnichar* chars, int count, SkGlyphID glyphs[]) const = 0;
    virtual int onCountGlyphs() const = 0;
    virtual int onGetUPEM() const = 0;
    virtual bool onGetKerningPairAdjustments(const SkGlyphID glyphs[], int count, int32_t adjustments[]) const;
    virtual void onGetFamilyName(SkString* familyName) const = 0;
    virtual bool onGetPostScriptName(SkString*) const = 0;
    virtual LocalizedStrings* onCreateFamilyNameIterator() const = 0;
    virtual int onGetTableTags(SkFontTableTag tags[]) const = 0;
    virtual size_t onGetTableData(SkFontTableTag, size_t offset, size_t length, void* data) const = 0;
    virtual sk_sp<SkData> onCopyTableData(SkFontTableTag) const;
    virtual bool onComputeBounds(SkRect*) const;
    virtual void* onGetCTFontRef() const { return nullptr; }

private:
    /** Retrieve detailed typeface metrics.  Used by the PDF backend.  */
    std::unique_ptr<SkAdvancedTypefaceMetrics> getAdvancedMetrics() const;
    friend class SkRandomTypeface; // getAdvancedMetrics
    friend class SkPDFFont;        // getAdvancedMetrics

    /** Style specifies the intrinsic style attributes of a given typeface */
    enum Style {
        kNormal = 0,
        kBold   = 0x01,
        kItalic = 0x02,
        // helpers
        kBoldItalic = 0x03
    };
    static SkFontStyle FromOldStyle(Style oldStyle);
    static SkTypeface* GetDefaultTypeface(Style style = SkTypeface::kNormal);

    friend class SkFontPriv;       // GetDefaultTypeface
    friend class SkPaintPriv;      // GetDefaultTypeface
    friend class SkFont;           // getGlyphToUnicodeMap

private:
    SkFontID            fUniqueID;
    SkFontStyle         fStyle;
    mutable SkRect      fBounds;
    mutable SkOnce      fBoundsOnce;
    bool                fIsFixedPitch;

    using INHERITED = SkWeakRefCnt;
};
#endif
