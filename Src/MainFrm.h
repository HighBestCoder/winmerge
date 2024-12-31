/////////////////////////////////////////////////////////////////////////////
//    WinMerge:  an interactive diff/merge utility
//    Copyright (C) 1997  Dean P. Grimm
//    SPDX-License-Identifier: GPL-2.0-or-later
/////////////////////////////////////////////////////////////////////////////
/** 
 * @file  MainFrm.h
 *
 * @brief Declaration file for CMainFrame
 *
 */
#pragma once

#include <vector>
#include <memory>
#include <optional>
#include "MyReBar.h"
#include "MenuBar.h"
#include "MDITabBar.h"
#include "BasicFlatStatusBar.h"
#include "PathContext.h"
#include "OptionsDef.h"
#include "OptionsMgr.h"
#include "FileOpenFlags.h"

class BCMenu;
class CDirView;
class COpenDoc;
struct IDirDoc;
class CDirDoc;
class CMergeDoc;
class CHexMergeDoc;
class CMergeEditView;
class SyntaxColors;
class LineFiltersList;
class TempFile;
struct FileLocation;
class DropHandler;
class CMainFrame;
class CImgMergeFrame;
class CWebPageDiffFrame;
class DirWatcher;

typedef std::shared_ptr<TempFile> TempFilePtr;

// typed lists (homogenous pointer lists)
typedef CTypedPtrList<CPtrList, COpenDoc *> OpenDocList;
typedef CTypedPtrList<CPtrList, CMergeDoc *> MergeDocList;
typedef CTypedPtrList<CPtrList, CDirDoc *> DirDocList;
typedef CTypedPtrList<CPtrList, CHexMergeDoc *> HexMergeDocList;

class PackingInfo;
class PrediffingInfo;
class CLanguageSelect;
struct IMergeDoc;

CMainFrame * GetMainFrame(); // access to the singleton main frame object

/**
 * @brief Frame class containing save-routines etc
 */
class CMainFrame : public CMDIFrameWnd
{
	friend CLanguageSelect;
	DECLARE_DYNAMIC(CMainFrame)
public:
	/**
	 * @brief Frame/View/Document types.
	 */
	enum FRAMETYPE
	{
		FRAME_FOLDER, /**< Folder compare frame. */
		FRAME_FILE, /**< File compare frame. */
		FRAME_HEXFILE, /**< Hex file compare frame. */
		FRAME_IMGFILE, /**< Image file compare frame. */
		FRAME_WEBPAGE, /**< Web page compare frame. */
		FRAME_OTHER, /**< No frame? */
	};

	struct OpenFileParams
	{
		virtual ~OpenFileParams() {}
	};

	struct OpenTextFileParams : public OpenFileParams
	{
		virtual ~OpenTextFileParams() {}
		int m_line = -1;
		int m_char = -1;
		String m_fileExt;
		String m_strSaveAsPath; /**< "3rd path" where output saved if given */
	};

    // OpenTableFileParams 结构体，继承自 OpenTextFileParams
    // 用于指定打开表格文件时的参数
    struct OpenTableFileParams : public OpenTextFileParams
    {
        // 虚析构函数，确保派生类能够正确释放资源
        virtual ~OpenTableFileParams() {}

        // 表格文件中的字段分隔符
        // 类型：std::optional<tchar_t>
        // 可选值：例如逗号（','）、制表符（'\t'）等
        // 作用：指定表格文件中用于分隔字段的字符
        // 示例：CSV 文件通常使用逗号作为分隔符
        std::optional<tchar_t> m_tableDelimiter;

        // 表格文件中的引用符
        // 类型：std::optional<tchar_t>
        // 可选值：例如双引号（'"'）、单引号（"'")等
        // 作用：指定表格文件中用于包围字段的字符，通常用于包含特殊字符（如分隔符或换行符）的字段
        // 示例：CSV 文件通常使用双引号作为引用符
        std::optional<tchar_t> m_tableQuote;

        // 是否允许在引用符内的字段中包含换行符
        // 类型：std::optional<bool>
        // 可选值：true（允许）、false（不允许）
        // 作用：控制是否允许在引用符内的字段中包含换行符
        // 示例：某些表格文件可能允许在引用符内的字段中包含多行文本
        std::optional<bool> m_tableAllowNewlinesInQuotes;
    };

    /// @brief OpenBinaryFileParams 结构体，继承自 OpenFileParams
    /// 用于指定打开二进制文件时的参数
    struct OpenBinaryFileParams : public OpenFileParams
    {
        /// @brief 虚析构函数，确保派生类能够正确释放资源
        virtual ~OpenBinaryFileParams() {}

        /// @brief 二进制文件的起始地址或偏移量
        /// @details 
        /// 类型：int
        /// 默认值：-1（表示未指定）
        /// 作用：指定从二进制文件的哪个位置开始读取或处理数据
        /// 示例：如果 m_address = 100，表示从文件的第 100 个字节开始处理
        /// 设计考虑：
        /// 二进制文件通常需要从特定位置读取数据（如文件头、特定区块等），
        /// 因此引入该参数以支持灵活的读取操作。
        int m_address = -1;

        /// @brief 保存输出文件的路径（“第三路径”）
        /// @details
        /// 类型：String
        /// 作用：如果指定了该路径，处理后的输出文件将保存到此路径
        /// 示例：如果 m_strSaveAsPath = "C:\\output.bin"，则输出文件将保存到该路径
        /// 设计考虑：
        /// 1. 二进制文件的处理通常需要生成独立的输出文件（如提取差异、转换格式等），
        ///    因此引入该参数以支持保存输出文件的功能。
        /// 2. 文本文件或表格文件通常直接在界面中操作并保存，不需要预先指定输出路径，
        ///    因此该参数未在其他 FileParams 结构体中引入。
        /// 3. 该参数的设计体现了 WinMerge 对二进制文件处理的特殊支持，
        ///    同时避免了不必要的复杂性扩展到其他文件类型。
        String m_strSaveAsPath; /**< "3rd path" where output saved if given */
    };

    /// @brief OpenImageFileParams 结构体，继承自 OpenFileParams
    /// 用于指定打开图像文件时的参数
    struct OpenImageFileParams : public OpenFileParams
    {
        /// @brief 虚析构函数，确保派生类能够正确释放资源
        virtual ~OpenImageFileParams() {}

        /// @brief 图像文件的水平坐标（X 轴、列）
        /// @details
        /// 类型：int
        /// 默认值：-1（表示未指定）
        /// 作用：指定图像文件中某个区域的水平起始位置
        /// 示例：如果 m_x = 100，表示从图像的 X 轴第 100 个像素开始处理
        /// 设计考虑：
        /// 1. 图像文件通常需要处理特定区域（如裁剪、缩放等），
        ///    因此引入 m_x 和 m_y 以支持对图像区域的精确定位。
        /// 2. 默认值为 -1，表示未指定，以便支持全图像处理。
        int m_x = -1;

        /// @brief 图像文件的垂直坐标（Y 轴、行）
        /// @details
        /// 类型：int
        /// 默认值：-1（表示未指定）
        /// 作用：指定图像文件中某个区域的垂直起始位置
        /// 示例：如果 m_y = 200，表示从图像的 Y 轴第 200 个像素开始处理
        /// 设计考虑：
        /// 1. 与 m_x 配合使用，共同定义图像处理的起始点。
        /// 2. 默认值为 -1，表示未指定，以便支持全图像处理。
        int m_y = -1;

        /// @brief 保存输出文件的路径（“第三路径”）
        /// @details
        /// 类型：String
        /// 作用：如果指定了该路径，处理后的输出文件将保存到此路径
        /// 示例：如果 m_strSaveAsPath = "C:\\output.png"，则输出文件将保存到该路径
        /// 设计考虑：
        /// 1. 图像文件的处理通常需要生成独立的输出文件（如裁剪后的图像、格式转换等），
        ///    因此引入该参数以支持保存输出文件的功能。
        /// 2. 与 OpenBinaryFileParams 类似，图像文件的处理结果通常需要保存到独立文件，
        ///    因此该参数在图像文件处理中也是必要的。
        /// 3. 该参数的设计体现了 WinMerge 对图像文件处理的特殊支持，
        ///    同时避免了不必要的复杂性扩展到其他文件类型。
        String m_strSaveAsPath; /**< "3rd path" where output saved if given */
    };

    /// @brief OpenWebPageParams 结构体，继承自 OpenFileParams
    /// 用于指定打开网页文件时的参数
    /// @details
    /// 设计考虑：
    /// 1. 网页文件（如 HTML）通常需要特殊的处理逻辑（如渲染、解析等），
    ///    因此单独定义该结构体以支持网页文件的处理。
    /// 2. 目前该结构体未定义额外参数，未来可以根据需求扩展。
    struct OpenWebPageParams : public OpenFileParams
    {
        /// @brief 虚析构函数，确保派生类能够正确释放资源
        virtual ~OpenWebPageParams() {}
    };

    /// @brief OpenAutoFileParams 结构体，继承自 OpenTableFileParams、OpenBinaryFileParams 和 OpenImageFileParams
    /// 用于自动识别文件类型并打开文件时的参数
    /// @details
    /// 设计考虑：
    /// 1. AutoFile 是指能够自动识别文件类型（如表格文件、二进制文件、图像文件等）并选择合适的方式打开的文件。
    /// 2. 该结构体通过多重继承整合了多种文件类型的参数，以便在自动识别文件类型时能够灵活处理。
    /// 3. 父类变量可能存在重叠，例如 m_strSaveAsPath 在 OpenBinaryFileParams 和 OpenImageFileParams 中均存在。
    ///    这种设计是为了确保在自动识别文件类型时，无论文件类型如何，都能支持保存输出文件的功能。
    /// 使用方式如下:
    /// ```
    ///     OpenAutoFileParams params;
    ///     // 指定访问 OpenBinaryFileParams 的 m_strSaveAsPath
    ///     params.OpenBinaryFileParams::m_strSaveAsPath = "binary_output.bin";
    ///     // 指定访问 OpenImageFileParams 的 m_strSaveAsPath
    ///     params.OpenImageFileParams::m_strSaveAsPath = "image_output.png";
    /// ```
    struct OpenAutoFileParams : public OpenTableFileParams, public OpenBinaryFileParams, public OpenImageFileParams
    {
        /// @brief 虚析构函数，确保派生类能够正确释放资源
        virtual ~OpenAutoFileParams() {}
    };

    /// @brief OpenFolderParams 结构体，继承自 OpenFileParams
    /// 用于指定打开文件夹时的参数
    /// @details
    /// 设计考虑：
    /// 1. 文件夹的处理通常需要特殊的逻辑（如遍历文件、过滤隐藏项等），
    ///    因此单独定义该结构体以支持文件夹的处理。
    /// 2. m_hiddenItems 用于指定需要隐藏的文件或文件夹列表，
    ///    以便在打开文件夹时过滤掉这些项。
    struct OpenFolderParams : public OpenFileParams
    {
        /// @brief 虚析构函数，确保派生类能够正确释放资源
        virtual ~OpenFolderParams() {}

        /// @brief 需要隐藏的文件或文件夹列表
        /// @details
        /// 类型：std::vector<String>
        /// 作用：指定在打开文件夹时需要隐藏的文件或文件夹列表
        /// 示例：如果 m_hiddenItems 包含 "temp"，则名为 "temp" 的文件或文件夹将被隐藏
        std::vector<String> m_hiddenItems;
    };

    CMainFrame();

// Attributes
public:	
    /// @brief 是否显示文件夹比较中的错误项
    /// @details
    /// 类型：bool
    /// 作用：控制是否在文件夹比较中显示错误项（如无法访问的文件或文件夹）。
    /// 示例：如果 m_bShowErrors = true，则显示错误项；否则隐藏错误项。
    bool m_bShowErrors;

    /// @brief MergeView 用户选择的字体
    /// @details
    /// 类型：LOGFONT
    /// 作用：存储用户在 MergeView（文件比较视图）中选择的字体设置。
    /// 示例：用户可以通过设置对话框更改 MergeView 的字体，更改后的字体信息会存储在此变量中。
    LOGFONT m_lfDiff;

    /// @brief DirView 用户选择的字体
    /// @details
    /// 类型：LOGFONT
    /// 作用：存储用户在 DirView（文件夹比较视图）中选择的字体设置。
    /// 示例：用户可以通过设置对话框更改 DirView 的字体，更改后的字体信息会存储在此变量中。
    LOGFONT m_lfDir;

    /// @brief 主窗口类的名称
    /// @details
    /// 类型：static const tchar_t[]
    /// 作用：存储主窗口类的名称，用于窗口注册和创建。
    /// 示例：szClassName 可能为 "WinMergeMainFrame"，用于标识主窗口类。
    static const tchar_t szClassName[];

// Operations
public:
    /// @brief 创建 DirView（文件夹比较视图）的上下文菜单
    /// @return 返回创建的菜单句柄（HMENU）
    /// @details
    /// 该函数用于生成文件夹比较视图的右键菜单，包含与文件夹操作相关的命令（如打开、复制、删除等）。
    HMENU NewDirViewMenu();

    /// @brief 创建 MergeView（文件比较视图）的上下文菜单
    /// @return 返回创建的菜单句柄（HMENU）
    /// @details
    /// 该函数用于生成文件比较视图的右键菜单，包含与文件比较操作相关的命令（如复制差异、合并、跳转到下一差异等）。
    HMENU NewMergeViewMenu();

    /// @brief 创建 HexMergeView（十六进制比较视图）的上下文菜单
    /// @return 返回创建的菜单句柄（HMENU）
    /// @details
    /// 该函数用于生成十六进制比较视图的右键菜单，包含与十六进制编辑操作相关的命令（如复制字节、跳转到偏移量等）。
    HMENU NewHexMergeViewMenu();

    /// @brief 创建 ImgMergeView（图像比较视图）的上下文菜单
    /// @return 返回创建的菜单句柄（HMENU）
    /// @details
    /// 该函数用于生成图像比较视图的右键菜单，包含与图像操作相关的命令（如放大、缩小、切换视图模式等）。
    HMENU NewImgMergeViewMenu();

    /// @brief 创建 WebPageDiffView（网页比较视图）的上下文菜单
    /// @return 返回创建的菜单句柄（HMENU）
    /// @details
    /// 该函数用于生成网页比较视图的右键菜单，包含与网页操作相关的命令（如刷新、查看源代码等）。
    HMENU NewWebPageDiffViewMenu();

    /// @brief 创建 OpenView（打开视图）的上下文菜单
    /// @return 返回创建的菜单句柄（HMENU）
    /// @details
    /// 该函数用于生成打开视图的右键菜单，包含与文件打开操作相关的命令（如打开文件、打开文件夹等）。
    HMENU NewOpenViewMenu();

    /// @brief 创建默认的上下文菜单
    /// @param ID 菜单的标识符（可选，默认为 0）
    /// @return 返回创建的菜单句柄（HMENU）
    /// @details
    /// 该函数用于生成默认的右键菜单，通常作为其他视图菜单的基菜单。
    /// 可以通过指定 ID 来创建特定的默认菜单。
    HMENU NewDefaultMenu(int ID = 0);

    /// @brief 获取 Prediffers（预比较工具）的子菜单
    /// @param mainMenu 主菜单的句柄
    /// @return 返回 Prediffers 子菜单的句柄（HMENU）
    /// @details
    /// 该函数用于从主菜单中提取 Prediffers 子菜单，Prediffers 是用于在比较前预处理文件的工具。
    HMENU GetPrediffersSubmenu(HMENU mainMenu);

    /// @brief 更新 Prediffers（预比较工具）菜单
    /// @details
    /// 该函数用于更新 Prediffers 菜单的内容，通常是在 Prediffers 工具列表发生变化时调用。
    void UpdatePrediffersMenu();

    /// @brief 打开文件或文件夹进行比较
    /// @param pFiles [IN] 文件或文件夹的路径上下文（PathContext）
    /// @param dwFlags [IN] 文件打开标志数组，控制文件打开的行为（如只读、二进制模式等）
    /// @param strDesc [IN] 文件描述数组，用于显示文件的描述信息
    /// @param sReportFile [IN] 报告文件的路径，用于保存比较结果
    /// @param bRecurse [IN] 是否递归比较子文件夹（可选，默认为 false）
    /// @param pDirDoc [IN] 指向当前文件夹文档的指针（可选）
    /// @param infoUnpacker [IN] 解包器信息，用于处理压缩文件（可选）
    /// @param infoPrediffer [IN] 预比较器信息，用于预处理文件内容（可选）
    /// @param nID [IN] 视图或窗口的标识符（可选，默认为 0）
    /// @param pOpenParams [IN] 文件打开参数，用于指定打开文件时的额外设置（可选）
    /// @return bool 成功返回 true，失败返回 false
    bool DoFileOrFolderOpen(const PathContext *pFiles = nullptr,
                            const fileopenflags_t dwFlags[] = nullptr,
                            const String strDesc[] = nullptr,
                            const String& sReportFile = _T(""),
                            std::optional<bool> bRecurse = false,
                            IDirDoc *pDirDoc = nullptr,
                            const PackingInfo * infoUnpacker = nullptr,
                            const PrediffingInfo * infoPrediffer = nullptr,
                            UINT nID = 0,
                            const OpenFileParams *pOpenParams = nullptr);

    /// @brief 打开文件进行比较
    /// @param nID [IN] 视图或窗口的标识符
    /// @param pFiles [IN] 文件的路径上下文（PathContext）
    /// @param dwFlags [IN] 文件打开标志数组，控制文件打开的行为（如只读、二进制模式等）
    /// @param strDesc [IN] 文件描述数组，用于显示文件的描述信息
    /// @param sReportFile [IN] 报告文件的路径，用于保存比较结果
    /// @param infoUnpacker [IN] 解包器信息，用于处理压缩文件（可选）
    /// @param infoPrediffer [IN] 预比较器信息，用于预处理文件内容（可选）
    /// @param pOpenParams [IN] 文件打开参数，用于指定打开文件时的额外设置（可选）
    /// @return bool 成功返回 true，失败返回 false
    bool DoFileOpen(UINT nID,
                    const PathContext* pFiles,
                    const fileopenflags_t dwFlags[] = nullptr,
                    const String strDesc[] = nullptr,
                    const String& sReportFile = _T(""),
                    const PackingInfo* infoUnpacker = nullptr,
                    const PrediffingInfo * infoPrediffer = nullptr,
                    const OpenFileParams *pOpenParams = nullptr);

    /// @brief 创建新文件进行比较
    /// @param nID [IN] 视图或窗口的标识符
    /// @param nPanes [IN] 窗格数量，指定比较的窗格数
    /// @param dwFlags [IN] 文件打开标志数组，控制文件打开的行为（如只读、二进制模式等）
    /// @param strDesc [IN] 文件描述数组，用于显示文件的描述信息
    /// @param infoPrediffer [IN] 预比较器信息，用于预处理文件内容（可选）
    /// @param pOpenParams [IN] 文件打开参数，用于指定打开文件时的额外设置（可选）
    /// @return bool 成功返回 true，失败返回 false
    bool DoFileNew(UINT nID,
                   int nPanes,
                   const fileopenflags_t dwFlags[] = nullptr,
                   const String strDesc[] = nullptr,
                   const PrediffingInfo * infoPrediffer = nullptr,
                   const OpenFileParams *pOpenParams = nullptr);

    /// @brief 打开冲突文件进行比较
    /// @param conflictFile [IN] 冲突文件的路径
    /// @param strDesc [IN] 文件描述数组，用于显示文件的描述信息（可选）
    /// @param checked [IN] 是否已检查冲突文件（可选，默认为 false）
    /// @return bool 成功返回 true，失败返回 false
    bool DoOpenConflict(const String& conflictFile,
                        const String strDesc[] = nullptr,
                        bool checked = false);

    /// @brief 打开剪贴板内容进行比较
    /// @param nID [IN] 视图或窗口的标识符（可选，默认为 0）
    /// @param nBuffers [IN] 缓冲区数量，指定比较的缓冲区数（可选，默认为 2）
    /// @param dwFlags [IN] 文件打开标志数组，控制文件打开的行为（如只读、二进制模式等）
    /// @param strDesc [IN] 文件描述数组，用于显示文件的描述信息（可选）
    /// @param infoUnpacker [IN] 解包器信息，用于处理压缩文件（可选）
    /// @param infoPrediffer [IN] 预比较器信息，用于预处理文件内容（可选）
    /// @param pOpenParams [IN] 文件打开参数，用于指定打开文件时的额外设置（可选）
    /// @return bool 成功返回 true，失败返回 false
    bool DoOpenClipboard(UINT nID = 0,
                         int nBuffers = 2,
                         const fileopenflags_t dwFlags[] = nullptr,
                         const String strDesc[] = nullptr,
                         const PackingInfo* infoUnpacker = nullptr,
                         const PrediffingInfo * infoPrediffer = nullptr,
                         const OpenFileParams* pOpenParams = nullptr);

    /// @brief 对文件进行自我比较
    /// @param nID [IN] 视图或窗口的标识符
    /// @param file [IN] 文件的路径
    /// @param strDesc [IN] 文件描述数组，用于显示文件的描述信息（可选）
    /// @param infoUnpacker [IN] 解包器信息，用于处理压缩文件（可选）
    /// @param infoPrediffer [IN] 预比较器信息，用于预处理文件内容（可选）
    /// @param pOpenParams [IN] 文件打开参数，用于指定打开文件时的额外设置（可选）
    /// @return bool 成功返回 true，失败返回 false
    bool DoSelfCompare(UINT nID,
                       const String& file,
                       const String strDesc[] = nullptr,
                       const PackingInfo* infoUnpacker = nullptr,
                       const PrediffingInfo * infoPrediffer = nullptr,
                       const OpenFileParams* pOpenParams = nullptr);

    /// @brief 显示自动合并文档
    /// @param nID [IN] 视图或窗口的标识符
    /// @param pDirDoc [IN] 指向当前文件夹文档的指针
    /// @param nFiles [IN] 文件数量
    /// @param fileloc [IN] 文件位置数组，指定文件的位置信息
    /// @param dwFlags [IN] 文件打开标志数组，控制文件打开的行为（如只读、二进制模式等）
    /// @param strDesc [IN] 文件描述数组，用于显示文件的描述信息
    /// @param sReportFile [IN] 报告文件的路径，用于保存比较结果（可选）
    /// @param infoUnpacker [IN] 解包器信息，用于处理压缩文件（可选）
    /// @param infoPrediffer [IN] 预比较器信息，用于预处理文件内容（可选）
    /// @param pOpenParams [IN] 文件打开参数，用于指定打开文件时的额外设置（可选）
    /// @return bool 成功返回 true，失败返回 false
    bool ShowAutoMergeDoc(UINT nID,
                          IDirDoc * pDirDoc,
                          int nFiles,
                          const FileLocation fileloc[],
                          const fileopenflags_t dwFlags[],
                          const String strDesc[],
                          const String& sReportFile = _T(""),
                          const PackingInfo * infoUnpacker = nullptr,
                          const PrediffingInfo * infoPrediffer = nullptr,
                          const OpenFileParams *pOpenParams = nullptr);

    /// @brief 显示合并文档
    /// @param nID [IN] 视图或窗口的标识符
    /// @param pDirDoc [IN] 指向当前文件夹文档的指针
    /// @param nFiles [IN] 文件数量
    /// @param fileloc [IN] 文件位置数组，指定文件的位置信息
    /// @param dwFlags [IN] 文件打开标志数组，控制文件打开的行为（如只读、二进制模式等）
    /// @param strDesc [IN] 文件描述数组，用于显示文件的描述信息
    /// @param sReportFile [IN] 报告文件的路径，用于保存比较结果（可选）
    /// @param infoUnpacker [IN] 解包器信息，用于处理压缩文件（可选）
    /// @param infoPrediffer [IN] 预比较器信息，用于预处理文件内容（可选）
    /// @param pOpenParams [IN] 文件打开参数，用于指定打开文件时的额外设置（可选）
    /// @return bool 成功返回 true，失败返回 false
    bool ShowMergeDoc(UINT nID,
                      IDirDoc * pDirDoc,
                      int nFiles,
                      const FileLocation fileloc[],
                      const fileopenflags_t dwFlags[],
                      const String strDesc[],
                      const String& sReportFile = _T(""),
                      const PackingInfo * infoUnpacker = nullptr,
                      const PrediffingInfo * infoPrediffer = nullptr,
                      const OpenFileParams *pOpenParams = nullptr);

    /// @brief 显示文本或表格合并文档
    /// @param table [IN] 是否为表格模式（可选）
    /// @param pDirDoc [IN] 指向当前文件夹文档的指针
    /// @param nFiles [IN] 文件数量
    /// @param fileloc [IN] 文件位置数组，指定文件的位置信息
    /// @param dwFlags [IN] 文件打开标志数组，控制文件打开的行为（如只读、二进制模式等）
    /// @param strDesc [IN] 文件描述数组，用于显示文件的描述信息
    /// @param sReportFile [IN] 报告文件的路径，用于保存比较结果（可选）
    /// @param infoUnpacker [IN] 解包器信息，用于处理压缩文件（可选）
    /// @param infoPrediffer [IN] 预比较器信息，用于预处理文件内容（可选）
    /// @param pOpenParams [IN] 文件打开参数，用于指定打开文件时的额外设置（可选）
    /// @return bool 成功返回 true，失败返回 false
    bool ShowTextOrTableMergeDoc(std::optional<bool> table,
                                 IDirDoc * pDirDoc,
                                 int nFiles,
                                 const FileLocation fileloc[],
                                 const fileopenflags_t dwFlags[],
                                 const String strDesc[],
                                 const String& sReportFile = _T(""),
                                 const PackingInfo * infoUnpacker = nullptr,
                                 const PrediffingInfo * infoPrediffer = nullptr,
                                 const OpenTextFileParams *pOpenParams = nullptr);

    /// @brief 显示文本合并文档
    /// @param pDirDoc [IN] 指向当前文件夹文档的指针
    /// @param nFiles [IN] 文件数量
    /// @param fileloc [IN] 文件位置数组，指定文件的位置信息
    /// @param dwFlags [IN] 文件打开标志数组，控制文件打开的行为（如只读、二进制模式等）
    /// @param strDesc [IN] 文件描述数组，用于显示文件的描述信息
    /// @param sReportFile [IN] 报告文件的路径，用于保存比较结果（可选）
    /// @param infoUnpacker [IN] 解包器信息，用于处理压缩文件（可选）
    /// @param infoPrediffer [IN] 预比较器信息，用于预处理文件内容（可选）
    /// @param pOpenParams [IN] 文件打开参数，用于指定打开文件时的额外设置（可选）
    /// @return bool 成功返回 true，失败返回 false
    bool ShowTextMergeDoc(IDirDoc * pDirDoc,
                          int nFiles,
                          const FileLocation fileloc[],
                          const fileopenflags_t dwFlags[],
                          const String strDesc[],
                          const String& sReportFile = _T(""),
                          const PackingInfo * infoUnpacker = nullptr,
                          const PrediffingInfo * infoPrediffer = nullptr,
                          const OpenTextFileParams *pOpenParams = nullptr);

    /// @brief 显示文本合并文档（基于文本内容）
    /// @param pDirDoc [IN] 指向当前文件夹文档的指针
    /// @param nBuffers [IN] 缓冲区数量，指定比较的缓冲区数
    /// @param text [IN] 文本内容数组，指定要比较的文本内容
    /// @param strDesc [IN] 文件描述数组，用于显示文件的描述信息
    /// @param strFileExt [IN] 文件扩展名，用于指定文件类型
    /// @param pOpenParams [IN] 文件打开参数，用于指定打开文件时的额外设置（可选）
    /// @return bool 成功返回 true，失败返回 false
    bool ShowTextMergeDoc(IDirDoc* pDirDoc,
                          int nBuffers,
                          const String text[],
                          const String strDesc[],
                          const String& strFileExt,
                          const OpenTextFileParams *pOpenParams = nullptr);

    /// @brief 显示表格合并文档
    /// @param pDirDoc [IN] 指向当前文件夹文档的指针
    /// @param nFiles [IN] 文件数量
    /// @param fileloc [IN] 文件位置数组，指定文件的位置信息
    /// @param dwFlags [IN] 文件打开标志数组，控制文件打开的行为（如只读、二进制模式等）
    /// @param strDesc [IN] 文件描述数组，用于显示文件的描述信息
    /// @param sReportFile [IN] 报告文件的路径，用于保存比较结果（可选）
    /// @param infoUnpacker [IN] 解包器信息，用于处理压缩文件（可选）
    /// @param infoPrediffer [IN] 预比较器信息，用于预处理文件内容（可选）
    /// @param pOpenParams [IN] 文件打开参数，用于指定打开文件时的额外设置（可选）
    /// @return bool 成功返回 true，失败返回 false
    bool ShowTableMergeDoc(IDirDoc * pDirDoc,
                           int nFiles,
                           const FileLocation fileloc[],
                           const fileopenflags_t dwFlags[],
                           const String strDesc[],
                           const String& sReportFile = _T(""),
                           const PackingInfo * infoUnpacker = nullptr,
                           const PrediffingInfo * infoPrediffer = nullptr,
                           const OpenTextFileParams *pOpenParams = nullptr);

    /// @brief 显示十六进制合并文档
    /// @param pDirDoc [IN] 指向当前文件夹文档的指针
    /// @param nFiles [IN] 文件数量
    /// @param fileloc [IN] 文件位置数组，指定文件的位置信息
    /// @param dwFlags [IN] 文件打开标志数组，控制文件打开的行为（如只读、二进制模式等）
    /// @param strDesc [IN] 文件描述数组，用于显示文件的描述信息
    /// @param sReportFile [IN] 报告文件的路径，用于保存比较结果（可选）
    /// @param infoUnpacker [IN] 解包器信息，用于处理压缩文件（可选）
    /// @param infoPrediffer [IN] 预比较器信息，用于预处理文件内容（可选）
    /// @param pOpenParams [IN] 文件打开参数，用于指定打开文件时的额外设置（可选）
    /// @return bool 成功返回 true，失败返回 false
    bool ShowHexMergeDoc(IDirDoc * pDirDoc,
                         int nFiles,
                         const FileLocation fileloc[],
                         const fileopenflags_t dwFlags[],
                         const String strDesc[],
                         const String& sReportFile = _T(""),
                         const PackingInfo * infoUnpacker = nullptr,
                         const PrediffingInfo * infoPrediffer = nullptr,
                         const OpenBinaryFileParams *pOpenParams = nullptr);

    /// @brief 显示图像合并文档
    /// @param pDirDoc [IN] 指向当前文件夹文档的指针
    /// @param nFiles [IN] 文件数量
    /// @param fileloc [IN] 文件位置数组，指定文件的位置信息
    /// @param dwFlags [IN] 文件打开标志数组，控制文件打开的行为（如只读、二进制模式等）
    /// @param strDesc [IN] 文件描述数组，用于显示文件的描述信息
    /// @param sReportFile [IN] 报告文件的路径，用于保存比较结果（可选）
    /// @param infoUnpacker [IN] 解包器信息，用于处理压缩文件（可选）
    /// @param infoPrediffer [IN] 预比较器信息，用于预处理文件内容（可选）
    /// @param pOpenParams [IN] 文件打开参数，用于指定打开文件时的额外设置（可选）
    /// @return bool 成功返回 true，失败返回 false
    bool ShowImgMergeDoc(IDirDoc * pDirDoc,
                         int nFiles,
                         const FileLocation fileloc[],
                         const fileopenflags_t dwFlags[],
                         const String strDesc[],
                         const String& sReportFile = _T(""),
                         const PackingInfo * infoUnpacker = nullptr,
                         const PrediffingInfo * infoPrediffer = nullptr,
                         const OpenImageFileParams *pOpenParams = nullptr);

    /// @brief 显示网页比较文档
    /// @param pDirDoc [IN] 指向当前文件夹文档的指针
    /// @param nFiles [IN] 文件数量
    /// @param fileloc [IN] 文件位置数组，指定文件的位置信息
    /// @param dwFlags [IN] 文件打开标志数组，控制文件打开的行为（如只读、二进制模式等）
    /// @param strDesc [IN] 文件描述数组，用于显示文件的描述信息
    /// @param sReportFile [IN] 报告文件的路径，用于保存比较结果（可选）
    /// @param infoUnpacker [IN] 解包器信息，用于处理压缩文件（可选）
    /// @param infoPrediffer [IN] 预比较器信息，用于预处理文件内容（可选）
    /// @param pOpenParams [IN] 文件打开参数，用于指定打开文件时的额外设置（可选）
    /// @return bool 成功返回 true，失败返回 false
    bool ShowWebDiffDoc(IDirDoc * pDirDoc,
                        int nFiles,
                        const FileLocation fileloc[],
                        const fileopenflags_t dwFlags[],
                        const String strDesc[],
                        const String& sReportFile = _T(""),
                        const PackingInfo * infoUnpacker = nullptr,
                        const PrediffingInfo * infoPrediffer = nullptr,
                        const OpenWebPageParams *pOpenParams = nullptr);

    /// @brief 更新标题栏和标签栏
    /// @details
    /// 该函数用于更新主窗口的标题栏和标签栏，以反映当前打开的文件或文件夹的状态。
    void UpdateTitleBarAndTabBar();

    /// @brief 更新资源
    /// @details
    /// 该函数用于更新应用程序的资源，例如重新加载菜单、工具栏等。
    void UpdateResources();

    /// @brief 应用差异选项
    /// @details
    /// 该函数用于应用用户设置的差异选项，例如忽略空格、忽略大小写等。
    void ApplyDiffOptions();

    /// @brief 选择过滤器
    /// @details
    /// 该函数用于选择文件比较时的过滤器，例如仅显示文本文件或二进制文件。
    void SelectFilter();

    /// @brief 开始闪烁
    /// @details
    /// 该函数用于使窗口或控件开始闪烁，通常用于吸引用户的注意。
    void StartFlashing();

    /// @brief 询问关闭确认
    /// @return bool 如果用户确认关闭，返回 true；否则返回 false。
    /// @details
    /// 该函数用于在关闭窗口或退出应用程序时，询问用户是否确认关闭。
    bool AskCloseConfirmation();

    /// @brief 获取框架类型
    /// @param pFrame [IN] 指向框架窗口的指针
    /// @return FRAMETYPE 返回框架的类型。
    /// @details
    /// 该函数用于获取指定框架窗口的类型。
    static FRAMETYPE GetFrameType(const CFrameWnd * pFrame);

    /// @brief 更新文档标题
    /// @details
    /// 该函数用于更新文档窗口的标题，以反映当前打开的文件或文件夹的状态。
    static void UpdateDocTitle();

    /// @brief 重新加载菜单
    /// @details
    /// 该函数用于重新加载主窗口的菜单，通常用于动态更新菜单项。
    static void ReloadMenu();

    /// @brief 追加插件菜单
    /// @param pMenu [IN] 指向菜单的指针
    /// @param filteredFilenames [IN] 过滤后的文件名
    /// @param events [IN] 插件事件列表
    /// @param addAllMenu [IN] 是否添加“全部”菜单项
    /// @param baseId [IN] 菜单项的基础 ID
    /// @details
    /// 该函数用于将插件菜单追加到指定菜单中。
    static void AppendPluginMenus(CMenu* pMenu,
                                  const String& filteredFilenames,
                                  const std::vector<std::wstring>& events,
                                  bool addAllMenu,
                                  unsigned baseId);

    /// @brief 通过菜单 ID 获取插件管道
    /// @param idSearch [IN] 要查找的菜单 ID
    /// @param events [IN] 插件事件列表
    /// @param baseId [IN] 菜单项的基础 ID
    /// @return String 返回插件管道的名称。
    /// @details
    /// 该函数用于通过菜单 ID 获取对应的插件管道名称。
    static String GetPluginPipelineByMenuId(unsigned idSearch, const std::vector<std::wstring>& events, unsigned baseId);

    /// @brief 获取拖放处理器
    /// @return DropHandler* 返回拖放处理器的指针。
    /// @details
    /// 该函数用于获取主窗口的拖放处理器。
    DropHandler *GetDropHandler() const { return m_pDropHandler; }

    /// @brief 获取子窗口数组
    /// @return const CTypedPtrArray<CPtrArray, CMDIChildWnd*>& 返回子窗口数组的引用。
    /// @details
    /// 该函数用于获取主窗口的子窗口数组。
    const CTypedPtrArray<CPtrArray, CMDIChildWnd*>& GetChildArray() const { return m_arrChild; }

    /// @brief 获取活动的合并文档
    /// @return IMergeDoc* 返回活动的合并文档的指针。
    /// @details
    /// 该函数用于获取当前活动的合并文档。
    IMergeDoc* GetActiveIMergeDoc();

    /// @brief 获取目录监视器
    /// @return DirWatcher* 返回目录监视器的指针。
    /// @details
    /// 该函数用于获取主窗口的目录监视器。
    DirWatcher* GetDirWatcher() { return m_pDirWatcher.get(); }

    /// @brief 监视文档
    /// @param pMergeDoc [IN] 指向合并文档的指针
    /// @details
    /// 该函数用于监视指定合并文档的目录变化。
    void WatchDocuments(IMergeDoc* pMergeDoc);

    /// @brief 取消监视文档
    /// @param pMergeDoc [IN] 指向合并文档的指针
    /// @details
    /// 该函数用于取消监视指定合并文档的目录变化。
    void UnwatchDocuments(IMergeDoc* pMergeDoc);

    /// @brief 获取菜单栏
    /// @return CMenuBar* 返回菜单栏的指针。
    /// @details
    /// 该函数用于获取主窗口的菜单栏。
    CMenuBar* GetMenuBar() { return &m_wndMenuBar; }

    /// @brief 获取工具栏
    /// @return CToolBar* 返回工具栏的指针。
    /// @details
    /// 该函数用于获取主窗口的工具栏。
    CToolBar* GetToolbar() { return &m_wndToolBar; }

    /// @brief 等待并执行消息循环
    /// @param completed [IN/OUT] 是否完成的标志
    /// @param ms [IN] 等待的时间（毫秒）
    /// @details
    /// 该函数用于在等待指定时间的同时执行消息循环，通常用于处理异步任务。
    static void WaitAndDoMessageLoop(bool& completed, int ms);

    ///. Overrides
    /// @brief 获取消息字符串
    /// @param nID [IN] 消息的标识符
    /// @param rMessage [OUT] 存储消息字符串的引用
    /// @details
    /// 该函数用于根据消息标识符获取对应的消息字符串，并将其存储在 rMessage 中。
    virtual void GetMessageString(UINT nID, CString& rMessage) const;

public:
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CMainFrame)

    /// @brief 激活框架窗口
    /// @param nCmdShow [IN] 窗口显示方式（可选，默认为 -1）
    /// @details
    /// 该函数用于激活框架窗口，并根据 nCmdShow 参数设置窗口的显示方式。
    virtual void ActivateFrame(int nCmdShow = -1);

    /// @brief 预处理消息
    /// @param pMsg [IN] 指向消息结构的指针
    /// @return BOOL 如果消息已处理，返回 TRUE；否则返回 FALSE。
    /// @details
    /// 该函数用于在消息分发到窗口之前对其进行预处理，例如处理快捷键或自定义消息。
    virtual BOOL PreTranslateMessage(MSG* pMsg);

    /// @brief 更新框架窗口标题
    /// @param bAddToTitle [IN] 是否将文档标题添加到窗口标题中
    /// @details
    /// 该函数用于更新框架窗口的标题，并根据 bAddToTitle 参数决定是否将文档标题添加到窗口标题中。
    virtual void OnUpdateFrameTitle(BOOL bAddToTitle);

    /// @brief 预处理窗口创建
    /// @param cs [IN/OUT] 指向窗口创建结构的引用
    /// @return BOOL 如果窗口创建可以继续，返回 TRUE；否则返回 FALSE。
    /// @details
    /// 该函数用于在窗口创建之前对窗口创建结构进行预处理，例如设置窗口样式或大小。
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

    /// @brief 析构函数
    /// @details
    /// 该函数用于释放 CMainFrame 对象占用的资源，并执行清理操作。
    virtual ~CMainFrame();

// Public implementation data
public:
    /// @brief 首次激活标志
    /// @details
    /// 该变量用于指示框架窗口是否是首次激活。如果是首次激活，窗口的位置和大小将从注册表中读取。
    bool m_bFirstTime; /**< If first time frame activated, get pos from reg */

// Implementation data
protected:
    /// @brief 状态栏
    /// @details
    /// 该成员变量表示主窗口的状态栏，用于显示状态信息（如行号、列号等）。
    CBasicFlatStatusBar  m_wndStatusBar;

    /// @brief ReBar 控件
    /// @details
    /// 该成员变量表示主窗口的 ReBar 控件，用于管理工具栏、菜单栏等可停靠控件。
    CMyReBar m_wndReBar;

    /// @brief 菜单栏
    /// @details
    /// 该成员变量表示主窗口的菜单栏，用于显示和管理菜单项。
    CMenuBar m_wndMenuBar;

    /// @brief 工具栏
    /// @details
    /// 该成员变量表示主窗口的工具栏，用于提供常用操作的快捷按钮。
    CToolBar m_wndToolBar;

    /// @brief 标签栏
    /// @details
    /// 该成员变量表示主窗口的标签栏，用于管理多个子窗口或文档的标签。
    CMDITabBar m_wndTabBar;

    /// @brief 子窗口数组
    /// @details
    /// 该成员变量是一个指针数组，用于存储主窗口的所有子窗口（如比较窗口、合并窗口等）。
    CTypedPtrArray<CPtrArray, CMDIChildWnd*> m_arrChild;

    /// @brief 自定义 MDI 客户端窗口类
    /// @details
    /// 该类继承自 CWnd，用于调整 MDI（多文档界面）客户端窗口的行为。
    /// 主要目的是减少窗口在最大化状态下的闪烁，并优化 MDI 子窗口的菜单和重绘逻辑。
    /// 这样设计的原因：
    /// 1. 减少闪烁：在 MDI 子窗口创建或激活时，禁用重绘以减少视觉上的闪烁。
    /// 2. 优化菜单管理：在 MDI 子窗口设置菜单时，动态更新主窗口的菜单栏状态。
    /// 3. 定时器控制：通过定时器延迟重绘操作，确保窗口状态稳定后再进行重绘。
    class CMDIClient : public CWnd
    {
        /// @brief 重绘定时器 ID
        /// @details
        /// 用于标识重绘操作的定时器 ID。
        static UINT_PTR const m_nRedrawTimer = 1612;

        /// @brief 窗口消息处理函数
        /// @param message [IN] 消息类型
        /// @param wParam [IN] 消息参数
        /// @param lParam [IN] 消息参数
        /// @return LRESULT 消息处理结果
        /// @details
        /// 该函数用于处理 MDI 客户端窗口的消息，包括窗口创建、激活、菜单设置和定时器事件。
        /// 通过重写此函数，可以自定义 MDI 客户端窗口的行为。
        virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
        {
            switch (message)
            {
            case WM_MDICREATE:
            case WM_MDIACTIVATE:
            {
                // 在最大化状态下，禁用重绘以减少闪烁
                BOOL bMaximized;
                HWND hwndActive = reinterpret_cast<HWND>(SendMessage(WM_MDIGETACTIVE, 0, reinterpret_cast<LPARAM>(&bMaximized)));
                if ((bMaximized || (message == WM_MDICREATE && !hwndActive)) &&
                    SetTimer(m_nRedrawTimer, USER_TIMER_MINIMUM, nullptr))
                {
                    SetRedraw(FALSE);
                }
                break;
            }
            case WM_MDISETMENU:
                // 设置菜单时，更新主窗口的菜单栏状态
                GetMainFrame()->SetMenuBarState(AFX_MBS_HIDDEN);
                GetMainFrame()->GetMenuBar()->AttachMenu(CMenu::FromHandle(reinterpret_cast<HMENU>(wParam)));
                return TRUE;
                break;
            case WM_TIMER:
                // 定时器触发时，重新启用重绘并刷新窗口
                if (wParam == m_nRedrawTimer)
                {
                    KillTimer(m_nRedrawTimer);
                    SetRedraw(TRUE);
                    RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE);
                }
                break;
            }
            return CWnd::WindowProc(message, wParam, lParam);
        }
    } m_wndMDIClient;

    /// @brief 枚举类型，定义不同视图的菜单类型
    /// @details
    /// 该枚举用于标识不同视图（如合并视图、文件夹视图、十六进制视图等）对应的菜单类型。
    /// 这样设计的原因是为了方便根据当前视图动态加载和切换菜单，提升用户体验。
    enum
    {
        MENU_DEFAULT,         ///< 默认菜单
        MENU_MERGEVIEW,       ///< 合并视图菜单
        MENU_DIRVIEW,         ///< 文件夹视图菜单
        MENU_HEXMERGEVIEW,    ///< 十六进制合并视图菜单
        MENU_IMGMERGEVIEW,    ///< 图像合并视图菜单
        MENU_WEBPAGEDIFFVIEW, ///< 网页差异视图菜单
        MENU_OPENVIEW,        ///< 打开视图菜单
        MENU_COUNT,           ///< 菜单类型总数，新增菜单类型需在此之前添加
    };

    /// @brief 枚举类型，定义菜单适用的框架类型
    /// @details
    /// 该枚举用于标识菜单适用的框架类型（如主框架、文件比较框架、文件夹比较框架等）。
    /// 这样设计的原因是为了根据不同的框架类型动态调整菜单内容，确保菜单功能与当前上下文一致。
    enum
    {
        MENU_MAINFRM = 0x000001,   ///< 主框架菜单
        MENU_FILECMP = 0x000002,   ///< 文件比较框架菜单
        MENU_FOLDERCMP = 0x000004, ///< 文件夹比较框架菜单
        MENU_ALL = MENU_MAINFRM | MENU_FILECMP | MENU_FOLDERCMP ///< 所有框架菜单
    };

    /// @brief 枚举类型，定义自动重新加载修改文件的模式
    /// @details
    /// 该枚举用于定义自动重新加载修改文件的模式（如禁用、窗口激活时重新加载、立即重新加载）。
    /// 这样设计的原因是为了提供灵活的文件管理选项，满足不同用户的需求。
    enum
    {
        AUTO_RELOAD_MODIFIED_FILES_DISABLED,           ///< 禁用自动重新加载
        AUTO_RELOAD_MODIFIED_FILES_ONWINDOWACTIVATED,  ///< 窗口激活时重新加载
        AUTO_RELOAD_MODIFIED_FILES_IMMEDIATELY         ///< 立即重新加载
    };

    /// @brief 结构体，用于关联菜单项、图标和适用的菜单类型
    /// @details
    /// 该结构体用于将菜单项、图标和适用的菜单类型关联起来，方便动态加载和显示菜单项。
    /// 这样设计的原因是为了实现菜单项和图标的统一管理，减少代码冗余。
    struct MENUITEM_ICON
    {
        int menuitemID;   ///< 菜单项的 ID
        int iconResID;    ///< 图标的资源 ID
        int menusToApply; ///< 适用的菜单类型
    };

    /// @brief 菜单项图标数组
    /// @details
    /// 该数组存储了所有菜单项及其对应的图标和适用的菜单类型。
    /// 这样设计的原因是为了集中管理菜单项和图标，方便动态加载和更新菜单。
    static const MENUITEM_ICON m_MenuIcons[];

    /// @brief 不同视图的菜单数组
    /// @details
    /// 该数组存储了不同视图（如合并视图、文件夹视图等）对应的菜单。
    /// 这样设计的原因是为了根据当前视图动态加载和切换菜单，提升用户体验。
    std::unique_ptr<BCMenu> m_pMenus[MENU_COUNT];

    /// @brief 图像菜单
    /// @details
    /// 该成员变量表示图像视图的菜单。
    /// 这样设计的原因是为了提供图像视图的专用菜单，方便用户操作。
    std::unique_ptr<BCMenu> m_pImageMenu;

    /// @brief 网页菜单
    /// @details
    /// 该成员变量表示网页差异视图的菜单。
    /// 这样设计的原因是为了提供网页差异视图的专用菜单，方便用户操作。
    std::unique_ptr<BCMenu> m_pWebPageMenu;

    /// @brief 临时文件列表
    /// @details
    /// 该成员变量存储了可能需要的临时文件。
    /// 这样设计的原因是为了在需要时快速访问和清理临时文件，避免资源泄漏。
    std::vector<TempFilePtr> m_tempFiles;

    /// @brief 拖放处理器
    /// @details
    /// 该成员变量表示拖放处理器，用于处理文件拖放操作。
    /// 这样设计的原因是为了支持文件拖放功能，提升用户体验。
    DropHandler *m_pDropHandler;

    /// @brief 目录监视器
    /// @details
    /// 该成员变量表示目录监视器，用于监视目录变化。
    /// 这样设计的原因是为了在目录内容发生变化时自动更新视图，提升用户体验。
    std::unique_ptr<DirWatcher> m_pDirWatcher;

    /// @brief 标签栏是否显示在标题栏中的标志
    /// @details
    /// 该成员变量表示标签栏是否显示在标题栏中。
    /// 这样设计的原因是为了提供灵活的界面布局选项，满足不同用户的需求。
    std::optional<bool> m_bTabsOnTitleBar;

    // Generated message map functions
protected:
    //{{AFX_MSG(CMainFrame)
    /// @brief 处理 WM_MEASUREITEM 消息，用于自定义菜单项或控件的尺寸
    afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);

    /// @brief 处理 WM_MENUCHAR 消息，用于处理菜单字符输入
    afx_msg LRESULT OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu);

    /// @brief 处理 WM_INITMENUPOPUP 消息，用于初始化弹出菜单
    afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);

    /// @brief 处理 WM_CREATE 消息，用于窗口创建时的初始化
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

    /// @brief 处理 ID_FILE_OPEN 命令，用于打开文件
    afx_msg void OnFileOpen();

    /// @brief 处理 ID_HELP_GNU_LICENSE 命令，用于显示 GNU 许可证信息
    afx_msg void OnHelpGnulicense();

    /// @brief 处理 ID_OPTIONS 命令，用于打开选项对话框
    afx_msg void OnOptions();

    /// @brief 处理 ID_VIEW_SELECTFONT 命令，用于选择字体
    afx_msg void OnViewSelectfont();

    /// @brief 处理 ID_VIEW_USEDEFAULTFONT 命令，用于恢复默认字体
    afx_msg void OnViewUsedefaultfont();

    /// @brief 处理 ID_HELP_CONTENTS 命令，用于显示帮助内容
    afx_msg void OnHelpContents();

    /// @brief 处理 WM_CLOSE 消息，用于关闭窗口
    afx_msg void OnClose();

    /// @brief 处理 ID_TOOLS_GENERATE_PATCH 命令，用于生成补丁文件
    afx_msg void OnToolsGeneratePatch();

    /// @brief 处理 WM_DROPFILES 消息，用于处理文件拖放操作
    afx_msg void OnDropFiles(const std::vector<String>& files);

    /// @brief 处理 ID_PLUGIN_UNPACK_MODE 命令的 UI 更新，用于更新插件解包模式的菜单项状态
    afx_msg void OnUpdatePluginUnpackMode(CCmdUI* pCmdUI);

    /// @brief 处理 ID_PLUGIN_UNPACK_MODE 命令，用于设置插件解包模式
    afx_msg void OnPluginUnpackMode(UINT nID);

    /// @brief 处理 ID_PLUGIN_PREDIFF_MODE 命令的 UI 更新，用于更新插件预比较模式的菜单项状态
    afx_msg void OnUpdatePluginPrediffMode(CCmdUI* pCmdUI);

    /// @brief 处理 ID_PLUGIN_PREDIFF_MODE 命令，用于设置插件预比较模式
    afx_msg void OnPluginPrediffMode(UINT nID);

    /// @brief 处理插件相关菜单的 UI 更新，用于更新插件相关菜单项的状态
    afx_msg void OnUpdatePluginRelatedMenu(CCmdUI* pCmdUI);

    /// @brief 处理 ID_PLUGIN_RELOAD 命令，用于重新加载插件
    afx_msg void OnReloadPlugins();

    /// @brief 处理 ID_SAVE_CONFIG_DATA 命令，用于保存配置数据
    afx_msg void OnSaveConfigData();

    /// @brief 处理 ID_FILE_NEW 命令，用于创建新文件比较任务
    template <int nFiles, unsigned nID>
    afx_msg void OnFileNew() { DoFileNew(nID, nFiles); }

    /// @brief 处理 ID_TOOLS_FILTERS 命令，用于打开过滤器对话框
    afx_msg void OnToolsFilters();

    /// @brief 处理 ID_VIEW_STATUS_BAR 命令，用于显示或隐藏状态栏
    afx_msg void OnViewStatusBar();

    /// @brief 处理 ID_VIEW_TAB_BAR 命令的 UI 更新，用于更新标签栏菜单项的状态
    afx_msg void OnUpdateViewTabBar(CCmdUI* pCmdUI);

    /// @brief 处理 ID_VIEW_TAB_BAR 命令，用于显示或隐藏标签栏
    afx_msg void OnViewTabBar();

    /// @brief 处理 ID_VIEW_TAB_BAR_ON_TITLE_BAR 命令的 UI 更新，用于更新标签栏是否显示在标题栏的菜单项状态
    afx_msg void OnUpdateViewTabBarOnTitleBar(CCmdUI* pCmdUI);

    /// @brief 处理 ID_VIEW_TAB_BAR_ON_TITLE_BAR 命令，用于设置标签栏是否显示在标题栏
    afx_msg void OnViewTabBarOnTitleBar();

    /// @brief 处理 ID_RESIZE_PANES 命令的 UI 更新，用于更新调整窗格大小菜单项的状态
    afx_msg void OnUpdateResizePanes(CCmdUI* pCmdUI);

    /// @brief 处理 ID_RESIZE_PANES 命令，用于调整窗格大小
    afx_msg void OnResizePanes();

    /// @brief 处理 ID_FILE_OPEN_PROJECT 命令，用于打开项目
    afx_msg void OnFileOpenProject();

    /// @brief 处理 WM_COPYDATA 消息，用于接收其他进程发送的数据
    afx_msg LRESULT OnCopyData(WPARAM wParam, LPARAM lParam);

    /// @brief 处理自定义消息 WM_USER1，用于自定义逻辑
    afx_msg LRESULT OnUser1(WPARAM wParam, LPARAM lParam);

    /// @brief 处理 ID_WINDOW_CLOSE_ALL 命令，用于关闭所有子窗口
    afx_msg void OnWindowCloseAll();

    /// @brief 处理 ID_WINDOW_CLOSE_ALL 命令的 UI 更新，用于更新关闭所有子窗口菜单项的状态
    afx_msg void OnUpdateWindowCloseAll(CCmdUI* pCmdUI);

    /// @brief 处理 ID_SAVE_PROJECT 命令，用于保存项目
    afx_msg void OnSaveProject();

    /// @brief 处理 WM_ACTIVATEAPP 消息，用于应用程序激活或失活时的处理
    afx_msg void OnActivateApp(BOOL bActive, DWORD dwThreadID);

    /// @brief 处理 WM_NCCALCSIZE 消息，用于计算非客户区的大小
    afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);

    /// @brief 处理 WM_SIZE 消息，用于窗口大小改变时的处理
    afx_msg void OnSize(UINT nType, int cx, int cy);

    /// @brief 处理工具栏按钮大小改变的消息，用于调整工具栏按钮大小
    afx_msg void OnToolbarSize(UINT id);

    /// @brief 处理工具栏按钮大小改变的 UI 更新，用于更新工具栏按钮大小菜单项的状态
    afx_msg void OnUpdateToolbarSize(CCmdUI* pCmdUI);

    /// @brief 处理 TTN_NEEDTEXT 通知，用于显示工具栏按钮的工具提示
    afx_msg BOOL OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult);

    /// @brief 处理 ID_HELP_RELEASENOTES 命令，用于显示版本发布说明
    afx_msg void OnHelpReleasenotes();

    /// @brief 处理 ID_HELP_TRANSLATIONS 命令，用于显示翻译信息
    afx_msg void OnHelpTranslations();

    /// @brief 处理 ID_FILE_OPEN_CONFLICT 命令，用于打开冲突文件
    afx_msg void OnFileOpenConflict();

    /// @brief 处理 ID_FILE_OPEN_CLIPBOARD 命令，用于打开剪贴板内容
    afx_msg void OnFileOpenClipboard();

    /// @brief 处理 ID_PLUGINS_LIST 命令，用于显示插件列表
    afx_msg void OnPluginsList();

    /// @brief 处理插件名称的 UI 更新，用于更新插件名称菜单项的状态
    afx_msg void OnUpdatePluginName(CCmdUI* pCmdUI);

    /// @brief 处理状态栏数字的 UI 更新，用于更新状态栏数字显示
    afx_msg void OnUpdateStatusNum(CCmdUI* pCmdUI);

    /// @brief 处理工具栏按钮下拉菜单的通知，用于显示工具栏按钮的下拉菜单
    afx_msg void OnToolbarButtonDropDown(NMHDR* pNMHDR, LRESULT* pResult);

    /// @brief 处理菜单栏按钮下拉菜单的通知，用于显示菜单栏按钮的下拉菜单
    afx_msg void OnMenubarButtonDropDown(NMHDR* pNMHDR, LRESULT* pResult);

    /// @brief 处理 ID_DIFF_WHITESPACE 命令，用于设置是否忽略空白字符
    afx_msg void OnDiffWhitespace(UINT nID);

    /// @brief 处理 ID_DIFF_WHITESPACE 命令的 UI 更新，用于更新忽略空白字符菜单项的状态
    afx_msg void OnUpdateDiffWhitespace(CCmdUI* pCmdUI);

    /// @brief 处理 ID_DIFF_IGNORE_BLANK_LINES 命令，用于设置是否忽略空行
    afx_msg void OnDiffIgnoreBlankLines();

    /// @brief 处理 ID_DIFF_IGNORE_BLANK_LINES 命令的 UI 更新，用于更新忽略空行菜单项的状态
    afx_msg void OnUpdateDiffIgnoreBlankLines(CCmdUI* pCmdUI);

    /// @brief 处理 ID_DIFF_IGNORE_CASE 命令，用于设置是否忽略大小写
    afx_msg void OnDiffIgnoreCase();

    /// @brief 处理 ID_DIFF_IGNORE_CASE 命令的 UI 更新，用于更新忽略大小写菜单项的状态
    afx_msg void OnUpdateDiffIgnoreCase(CCmdUI* pCmdUI);

    /// @brief 处理 ID_DIFF_IGNORE_NUMBERS 命令，用于设置是否忽略数字
    afx_msg void OnDiffIgnoreNumbers();

    /// @brief 处理 ID_DIFF_IGNORE_NUMBERS 命令的 UI 更新，用于更新忽略数字菜单项的状态
    afx_msg void OnUpdateDiffIgnoreNumbers(CCmdUI* pCmdUI);

    /// @brief 处理 ID_DIFF_IGNORE_EOL 命令，用于设置是否忽略行尾符
    afx_msg void OnDiffIgnoreEOL();

    /// @brief 处理 ID_DIFF_IGNORE_EOL 命令的 UI 更新，用于更新忽略行尾符菜单项的状态
    afx_msg void OnUpdateDiffIgnoreEOL(CCmdUI* pCmdUI);

    /// @brief 处理 ID_DIFF_IGNORE_CP 命令，用于设置是否忽略代码页
    afx_msg void OnDiffIgnoreCP();

    /// @brief 处理 ID_DIFF_IGNORE_CP 命令的 UI 更新，用于更新忽略代码页菜单项的状态
    afx_msg void OnUpdateDiffIgnoreCP(CCmdUI* pCmdUI);

    /// @brief 处理 ID_DIFF_IGNORE_COMMENTS 命令，用于设置是否忽略注释
    afx_msg void OnDiffIgnoreComments();

    /// @brief 处理 ID_DIFF_IGNORE_COMMENTS 命令的 UI 更新，用于更新忽略注释菜单项的状态
    afx_msg void OnUpdateDiffIgnoreComments(CCmdUI* pCmdUI);

    /// @brief 处理 ID_DIFF_IGNORE_MISSING_TRAILING_EOL 命令，用于设置是否忽略缺失的行尾符
    afx_msg void OnDiffIgnoreMissingTrailingEol();

    /// @brief 处理 ID_DIFF_IGNORE_MISSING_TRAILING_EOL 命令的 UI 更新，用于更新忽略缺失的行尾符菜单项的状态
    afx_msg void OnUpdateDiffIgnoreMissingTrailingEol(CCmdUI* pCmdUI);

    /// @brief 处理 ID_INCLUDE_SUBFOLDERS 命令，用于设置是否包含子文件夹
    afx_msg void OnIncludeSubfolders();

    /// @brief 处理 ID_INCLUDE_SUBFOLDERS 命令的 UI 更新，用于更新包含子文件夹菜单项的状态
    afx_msg void OnUpdateIncludeSubfolders(CCmdUI* pCmdUI);

    /// @brief 处理 ID_COMPARE_METHOD 命令，用于设置比较方法
    afx_msg void OnCompareMethod(UINT nID);

    /// @brief 处理 ID_COMPARE_METHOD 命令的 UI 更新，用于更新比较方法菜单项的状态
    afx_msg void OnUpdateCompareMethod(CCmdUI* pCmdUI);

    /// @brief 处理 ID_MRUs 命令，用于打开最近使用的文件
    afx_msg void OnMRUs(UINT nID);

    /// @brief 处理 ID_NO_MRUs 命令的 UI 更新，用于更新最近使用文件菜单项的状态
    afx_msg void OnUpdateNoMRUs(CCmdUI* pCmdUI);

    /// @brief 处理 ID_FIRST_FILE 命令，用于跳转到第一个文件
    afx_msg void OnFirstFile();

    /// @brief 处理 ID_FIRST_FILE 命令的 UI 更新，用于更新跳转到第一个文件菜单项的状态
    afx_msg void OnUpdateFirstFile(CCmdUI* pCmdUI);

    /// @brief 处理 ID_PREV_FILE 命令，用于跳转到上一个文件
    afx_msg void OnPrevFile();

    /// @brief 处理 ID_PREV_FILE 命令的 UI 更新，用于更新跳转到上一个文件菜单项的状态
    afx_msg void OnUpdatePrevFile(CCmdUI* pCmdUI);

    /// @brief 处理 ID_NEXT_FILE 命令，用于跳转到下一个文件
    afx_msg void OnNextFile();

    /// @brief 处理 ID_NEXT_FILE 命令的 UI 更新，用于更新跳转到下一个文件菜单项的状态
    afx_msg void OnUpdateNextFile(CCmdUI* pCmdUI);

    /// @brief 处理 ID_LAST_FILE 命令，用于跳转到最后一个文件
    afx_msg void OnLastFile();

    /// @brief 处理 ID_LAST_FILE 命令的 UI 更新，用于更新跳转到最后一个文件菜单项的状态
    afx_msg void OnUpdateLastFile(CCmdUI* pCmdUI);

    /// @brief 处理 WM_TIMER 消息，用于定时器触发时的处理
    afx_msg void OnTimer(UINT_PTR nIDEvent);

    /// @brief 处理 WM_DESTROY 消息，用于窗口销毁时的清理操作
    afx_msg void OnDestroy();

    /// @brief 处理快捷键退出命令，用于退出应用程序
    afx_msg void OnAccelQuit();

    /// @brief 处理自定义消息 WM_CHILD_FRAME_ADDED，用于子窗口添加时的处理
    afx_msg LRESULT OnChildFrameAdded(WPARAM wParam, LPARAM lParam);

    /// @brief 处理自定义消息 WM_CHILD_FRAME_REMOVED，用于子窗口移除时的处理
    afx_msg LRESULT OnChildFrameRemoved(WPARAM wParam, LPARAM lParam);

    /// @brief 处理自定义消息 WM_CHILD_FRAME_ACTIVATE，用于子窗口激活时的处理
    afx_msg LRESULT OnChildFrameActivate(WPARAM wParam, LPARAM lParam);

    /// @brief 处理自定义消息 WM_CHILD_FRAME_ACTIVATED，用于子窗口激活完成时的处理
    afx_msg LRESULT OnChildFrameActivated(WPARAM wParam, LPARAM lParam);

    /// @brief 处理菜单栏菜单项的 UI 更新，用于更新菜单栏菜单项的状态
    afx_msg void OnUpdateMenuBarMenuItem(CCmdUI* pCmdUI);

    /// @brief 处理 ID_VIEW_MENU_BAR 命令，用于显示或隐藏菜单栏
    afx_msg void OnViewMenuBar();

    /// @brief 处理 ID_VIEW_MENU_BAR 命令的 UI 更新，用于更新显示或隐藏菜单栏菜单项的状态
    afx_msg void OnUpdateViewMenuBar(CCmdUI* pCmdUI);

    /// @brief 处理 WM_SYSCOMMAND 消息，用于系统命令的处理
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:
    /// @brief 将项目添加到最近使用文件列表
    void addToMru(const tchar_t* szItem, const tchar_t* szRegSubKey, UINT nMaxItems = 20);

    /// @brief 获取所有打开的文档列表
    OpenDocList &GetAllOpenDocs();

    /// @brief 获取所有合并文档列表
    MergeDocList &GetAllMergeDocs();

    /// @brief 获取所有文件夹比较文档列表
    DirDocList &GetAllDirDocs();

    /// @brief 获取所有十六进制合并文档列表
    HexMergeDocList &GetAllHexMergeDocs();

    /// @brief 获取所有图像合并框架列表
    std::vector<CImgMergeFrame *> GetAllImgMergeFrames();

    /// @brief 获取所有网页差异框架列表
    std::vector<CWebPageDiffFrame *> GetAllWebPageDiffFrames();

    /// @brief 更新指定框架的字体
    void UpdateFont(FRAMETYPE frame);

    /// @brief 创建工具栏
    BOOL CreateToolbar();

    /// @brief 获取活动的合并编辑视图
    CMergeEditView* GetActiveMergeEditView();

    /// @brief 加载工具栏图像
    void LoadToolbarImages();

    /// @brief 创建指定视图的菜单
    HMENU NewMenu(int view, int ID);

    /// @brief 如果文件较大，则比较文件
    bool CompareFilesIfFilesAreLarge(IDirDoc* pDirDoc, int nFiles, const FileLocation ifileloc[]);

    /// @brief 更新系统菜单
    void UpdateSystemMenu();

    /// @brief 存储长文本的宽字符数组
    std::unique_ptr<WCHAR[]> m_upszLongTextW;

    /// @brief 存储长文本的多字节字符数组
    std::unique_ptr<CHAR[]> m_upszLongTextA;
};
