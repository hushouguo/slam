// <auto-generated>
//     Generated by the protocol buffer compiler.  DO NOT EDIT!
//     source: common.proto
// </auto-generated>
#pragma warning disable 1591, 0612, 3021
#region Designer generated code

using pb = global::Google.Protobuf;
using pbc = global::Google.Protobuf.Collections;
using pbr = global::Google.Protobuf.Reflection;
using scg = global::System.Collections.Generic;
namespace Protocol {

  /// <summary>Holder for reflection information generated from common.proto</summary>
  public static partial class CommonReflection {

    #region Descriptor
    /// <summary>File descriptor for common.proto</summary>
    public static pbr::FileDescriptor Descriptor {
      get { return descriptor; }
    }
    private static pbr::FileDescriptor descriptor;

    static CommonReflection() {
      byte[] descriptorData = global::System.Convert.FromBase64String(
          string.Concat(
            "Cgxjb21tb24ucHJvdG8SCHByb3RvY29sKh4KBkdFTkRFUhIICgRNQUxFEAAS",
            "CgoGRkVNQUxFEAEqKAoFQ29vcmQSCQoFRlJPTlQQABIKCgZNSURETEUQARII",
            "CgRCQUNLEAJiBnByb3RvMw=="));
      descriptor = pbr::FileDescriptor.FromGeneratedCode(descriptorData,
          new pbr::FileDescriptor[] { },
          new pbr::GeneratedClrTypeInfo(new[] {typeof(global::Protocol.GENDER), typeof(global::Protocol.Coord), }, null));
    }
    #endregion

  }
  #region Enums
  public enum GENDER {
    [pbr::OriginalName("MALE")] Male = 0,
    [pbr::OriginalName("FEMALE")] Female = 1,
  }

  /// <summary>
  ///站位
  /// </summary>
  public enum Coord {
    [pbr::OriginalName("FRONT")] Front = 0,
    [pbr::OriginalName("MIDDLE")] Middle = 1,
    [pbr::OriginalName("BACK")] Back = 2,
  }

  #endregion

}

#endregion Designer generated code
