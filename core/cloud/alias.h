#ifndef _CORE_CLOUD_ALIAS_H
#define _CORE_CLOUD_ALIAS_H

// core/cloud/tokenalias.h
// 10/5/2011

#include "core/cloud/traits.h"
#include <QString>
#include <QList>
#include <QMetaType>

namespace Core { namespace Cloud {

  class Alias
  {
    typedef Alias Self;

    // - Types -
  public:
    typedef Traits::Language Language;
    typedef Traits::Type Type;

    enum AliasType {
      AT_Null = 0, // invalid
      AT_Name = 1,
      AT_Source = 2,
      AT_Tag = 3
    };

    enum AliasStatus {
      AS_Active = 0,
      AS_Deleted = -1,
      AS_Blocked = -2
    };

    enum AliasFlag {
    };

    // - Properties -

  private: qint32 type_;
  public:
    qint32 type() const                 { return type_; }
    void setType(qint32 t)              { type_ = t; }
    bool hasType() const                { return type_; }

  private: qint64 id_;
  public:
    qint64 id() const                   { return id_; }
    void setId(qint64 id)               { id_ = id; }
    bool hasId() const                  { return id_; }

  private: qint64 tokenId_;
  public:
    qint64 tokenId() const              { return tokenId_; }
    void setTokenId(qint64 tid)         { tokenId_ = tid; }
    bool hasTokenId() const             { return tokenId_; }

    ///  Used only in offline mode
  private: QString tokenDigest_;
  public:
    QString tokenDigest() const             { return tokenDigest_; }
    void setTokenDigest(const QString &hex) { tokenDigest_ = hex; }
    bool hasTokenDigest() const             { return !tokenDigest_.isNull(); }

  private: qint64 userId_;
  public:
    qint64 userId() const               { return userId_; }
    void setUserId(qint64 uid)          { userId_ = uid; }
    bool hasUserId() const              { return userId_; }

  private: int aliasType_;
  public:
    int aliasType() const               { return aliasType_; }
    void setAliasType(int at)           { aliasType_ = at; }
    bool hasAliasType() const           { return aliasType_; }

  private: qint32 status_;
  public:
    qint32 status() const               { return status_; }
    void setStatus(qint32 status)       { status_ = status; }

  private: quint64 flags_;
  public:
    quint64 flags() const               { return flags_; }
    void setFlags(quint64 flags)        { flags_ = flags; }

    bool hasFlag(quint64 bit) const     { return flags_ & bit; }

    void setFlag(quint64 bit, bool enabled = true)
    {
      if (enabled)
        flags_ |= bit;
      else
        flags_ &= ~bit;
    }

  private: qint32 language_;
  public:
    qint32 language() const            { return language_; }
    void setLanguage(qint32 lang)      { language_ = lang; }
    bool hasLanguage() const           { return language_; }

  private: QString text_;
  public:
    QString text() const                { return text_; }
    void setText(const QString &text)   { text_ = text; }
    bool hasText() const                { return !text_.isNull(); }

  private: qint64 updateTime_;
  public:
    qint64 updateTime() const           { return updateTime_; }
    void setUpdateTime(qint64 secs)     { updateTime_ = secs; }
    bool hasUpdateTime() const          { return updateTime_ > 0; }

  private: quint32 blessed_;
  public:
    quint32 blessedCount() const        { return blessed_; }
    void setBlessedCount(quint32 count) { blessed_ = count; }
    bool isBlessed() const              { return blessed_; }

  private: quint32 cursed_;
  public:
    quint32 cursedCount() const         { return cursed_; }
    void setCursedCount(quint32 count)  { cursed_ = count; }
    bool isCursed() const               { return cursed_; }

  private: quint32 blocked_;
  public:
    quint32 blockedCount() const         { return blocked_; }
    void setBlockedCount(quint32 count)  { blocked_ = count; }
    bool isBlocked() const               { return blocked_; }

    // - Constructions -
  public:
    Alias()
      : type_(0), id_(0), tokenId_(0), userId_(0), aliasType_(0),
        status_(0), flags_(0), language_(0), updateTime_(0),
        blessed_(0), cursed_(0), blocked_(0)
    { }

    bool isValid() const { return hasId() && hasType(); }

    void clear() { (*this) = Self(); }
  };

  typedef QList<Alias> AliasList;

} } // namespace Cloud, Core

using namespace Core::Cloud; // TO BE REMOVED!
Q_DECLARE_METATYPE(Alias);
Q_DECLARE_METATYPE(AliasList);

#endif // _CORE_CLOUD_ALIAS_H