
// MFCAppDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "MFCApp.h"
#include "MFCAppDlg.h"
#include "afxdialogex.h"
#include <codecvt>
#include <experimental/filesystem>
#include "h264_nal_parse.h"
#include "hevc_nal_parse.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMFCAppDlg 对话框

#define UM_FILE_PATH_CHANGED WM_USER + 0x0001

static std::wstring c2w(const std::string& str)
{
    setlocale(LC_ALL, "chs");
    size_t size = mbstowcs(nullptr, str.c_str(), 0) + 1;
    std::unique_ptr<wchar_t[]> dest(new wchar_t[size]);
    memset(dest.get(), 0, size);
    mbstowcs(dest.get(), str.c_str(), size);
    std::wstring result(dest.get(), size);
    setlocale(LC_ALL, "C");
    return result;
}

static std::string w2c(const std::wstring& str)
{
    setlocale(LC_ALL, "chs");
    size_t size = wcstombs(nullptr, str.c_str(), 0) + 1;
    std::unique_ptr<char[]> dest(new char[size]);
    memset(dest.get(), 0, size);

    wcstombs(dest.get(), str.c_str(), size);
    std::string result(dest.get(), size);
    setlocale(LC_ALL, "C");
    return result;
}


CMFCAppDlg::CMFCAppDlg(CWnd* pParent /*=NULL*/)
    : CDialogEx(IDD_MFCAPP_DIALOG, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCAppDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_NAL_VIEW, nal_unit_list_ctrl_);
    DDX_Control(pDX, IDC_EDT_HEX, nal_data_hex_);
}

void CMFCAppDlg::InitListCtrl()
{
    CRect list_ctrl_rt;
    nal_unit_list_ctrl_.GetWindowRect(list_ctrl_rt);
    int avage_width = list_ctrl_rt.Width() / 6;
    // add style
    nal_unit_list_ctrl_.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
    // add list head
    nal_unit_list_ctrl_.InsertColumn(0, L"No.", LVCFMT_LEFT, avage_width);
    nal_unit_list_ctrl_.InsertColumn(1, L"Offset", LVCFMT_LEFT, avage_width);
    nal_unit_list_ctrl_.InsertColumn(2, L"Length", LVCFMT_LEFT, avage_width);
    nal_unit_list_ctrl_.InsertColumn(3, L"Start Code", LVCFMT_LEFT, avage_width);
    nal_unit_list_ctrl_.InsertColumn(4, L"Nal Type", LVCFMT_LEFT, avage_width);
    nal_unit_list_ctrl_.InsertColumn(5, L"Info", LVCFMT_LEFT, avage_width);
}

DWORD __stdcall CMFCAppDlg::ParseNalThread(LPVOID param)
{
    CMFCAppDlg* obj = reinterpret_cast<CMFCAppDlg*>(param);

    if (!obj) return 1;
    obj->parse_fininsed_ = false;
    obj->StartParse();
    obj->parse_fininsed_ = true;
    return 0;
}

void CMFCAppDlg::StartParse()
{
    if (!nal_parse_) return;

    bool ret = nal_parse_->ProbeNal();

    if (ret) UpdateListCtrl();

}

void CMFCAppDlg::UpdateListCtrl()
{
    auto nals = nal_parse_->GetNals();
    int index = 0;
    for (auto iter = nals.begin(); iter != nals.end(); ++iter)
    {
        auto hex = nal::NalParse::Bin2HexString(iter->second->start_code);
        nal_unit_list_ctrl_.InsertItem(index, std::to_wstring(iter->first).c_str());
        nal_unit_list_ctrl_.SetItemText(index, 1, c2w(nal::NalParse::ToUpper(nal::NalParse::Number2HexString(iter->second->offset))).c_str());
        nal_unit_list_ctrl_.SetItemText(index, 2, std::to_wstring(iter->second->length).c_str());
        nal_unit_list_ctrl_.SetItemText(index, 3, c2w(nal::NalParse::ToUpper(nal::NalParse::Bin2HexString(iter->second->start_code))).c_str());
        nal_unit_list_ctrl_.SetItemText(index, 4, c2w(iter->second->type).c_str());
        nal_unit_list_ctrl_.SetItemText(index, 5, c2w(iter->second->slice_type).c_str());


        if (iter->second->slice_type == "VPS")
        {
            SetListCtrlItemColor(index, RGB(255, 0, 0));
        }
        if (iter->second->slice_type == "SPS")
        {
            SetListCtrlItemColor(index, RGB(0, 255, 0));
        }
        if (iter->second->slice_type == "PPS")
        {
            SetListCtrlItemColor(index, RGB(0, 0, 255));
        }
        if (iter->second->slice_type == "IDR")
        {
            SetListCtrlItemColor(index, RGB(255, 255, 0));
        }

        ++index;
    }
}

void CMFCAppDlg::DeleteItem(int first, int last)
{
    for (auto index = first; index <= last; ++index)
    {
        nal_unit_list_ctrl_.DeleteItem(first);
        DeleteListCtrlItemColor(index);
    }
}

void CMFCAppDlg::SetListCtrlItemColor(int item, COLORREF color)
{
    items_colors_.emplace(std::make_pair(item, color));
}

void CMFCAppDlg::DeleteListCtrlItemColor(int item)
{
    items_colors_.erase(item);
}

void CMFCAppDlg::InitEditCtrl()
{
    nal_data_hex_.SetMargins(10, 10);
}

BEGIN_MESSAGE_MAP(CMFCAppDlg, CDialogEx)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_DROPFILES()
    ON_WM_CLOSE()
    ON_MESSAGE(UM_FILE_PATH_CHANGED, &CMFCAppDlg::OnUmFilePathChanged)
    ON_BN_CLICKED(IDC_BTN_DEL_NAL, &CMFCAppDlg::OnBnClickedBtnDelNal)
    ON_BN_CLICKED(IDC_BTN_SAVE, &CMFCAppDlg::OnBnClickedBtnSave)
    ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_NAL_VIEW, &CMFCAppDlg::OnCustomdrawListNalView)
    ON_BN_CLICKED(IDCANCEL, &CMFCAppDlg::OnBnClickedCancel)
    ON_WM_WINDOWPOSCHANGING()
    ON_NOTIFY(NM_CLICK, IDC_LIST_NAL_VIEW, &CMFCAppDlg::OnClickListNalView)
END_MESSAGE_MAP()


// CMFCAppDlg 消息处理程序

BOOL CMFCAppDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
    //  执行此操作
    SetIcon(m_hIcon, TRUE);         // 设置大图标
    SetIcon(m_hIcon, FALSE);        // 设置小图标

    // TODO: 在此添加额外的初始化代码
    InitListCtrl();




    return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMFCAppDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // 用于绘制的设备上下文

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // 使图标在工作区矩形中居中
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // 绘制图标
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialogEx::OnPaint();
    }
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMFCAppDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}


void CMFCAppDlg::OnDropFiles(HDROP hDropInfo)
{
    if (!parse_fininsed_)
    {
        MessageBoxW(L"parsing ... ...", L"WARING", MB_ICONERROR);
        return;
    }
    int file_num = DragQueryFile(hDropInfo, -1, nullptr, 0);
    CString file_path;
    if (file_num)
    {
        WCHAR str_buffer[MAX_PATH]{ 0 };
        // 获取第一个文件
        DragQueryFile(hDropInfo, 0, str_buffer, MAX_PATH);
        file_path = CString(str_buffer);
    }

    DragFinish(hDropInfo);

    if (!file_path.IsEmpty())
    {
        SetWindowTextW(file_path);
        file_path_ = file_path;

        PostMessageW(UM_FILE_PATH_CHANGED, 0, 0);
    }

    CDialogEx::OnDropFiles(hDropInfo);
}


void CMFCAppDlg::OnClose()
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值

    CDialogEx::OnCancel();
}


afx_msg LRESULT CMFCAppDlg::OnUmFilePathChanged(WPARAM wParam, LPARAM lParam)
{
    using namespace std::experimental::filesystem;
    if (nal_parse_)
    {
        nal_parse_.reset();
        nal_unit_list_ctrl_.DeleteAllItems();
    }
    auto file_path = w2c(file_path_.GetBuffer());
    auto stream_type = nal::NalParse::GetStreamType(file_path);
    if (stream_type == DecodeFormat::H264)
    {
        nal_parse_.reset(new nal::H264NalParse(file_path));
    }
    else if (stream_type == DecodeFormat::HEVC)
    {
        nal_parse_.reset(new nal::HevcNalParse(file_path));
    }

    stream_type_ = stream_type;

    DWORD pid = 0;
    CreateThread(nullptr, 0, CMFCAppDlg::ParseNalThread, reinterpret_cast<LPVOID>(this), 0, &pid);
    return 0;
}


void CMFCAppDlg::OnBnClickedBtnDelNal()
{
    auto item_counts = nal_unit_list_ctrl_.GetItemCount();

    int min_index = -1, max_index = -1;

    for (auto index = 0; index < item_counts; ++index)
    {
        if (nal_unit_list_ctrl_.GetCheck(index))
        {
            if (min_index == -1)
            {
                min_index = max_index = index;
            }
            else
            {
                min_index = min(min_index, index);
                max_index = max(max_index, index);
            }
        }
    }

    TRACE(L"min index:%d, max index:%d\n", min_index, max_index);
    if (min_index == -1 && max_index == -1)
    {
        MessageBoxW(L"please check del nal range index.");
        return;
    }
    if (!nal_parse_)
    {
        MessageBoxW(L"not parse nal.");
        return;
    }

    if (!parse_fininsed_)
    {
        MessageBoxW(L"parsing nal ... ...");
        return;
    }

    this->GetDlgItem(IDC_BTN_DEL_NAL)->EnableWindow(FALSE);
    this->GetDlgItem(IDC_BTN_SAVE)->EnableWindow(FALSE);
    nal_parse_->DelRangeNalDdata(min_index, max_index + 1);
    this->GetDlgItem(IDC_BTN_DEL_NAL)->EnableWindow();
    this->GetDlgItem(IDC_BTN_SAVE)->EnableWindow();

    DeleteItem(min_index, max_index);
    TRACE(L"nal counts:%d\n", nal_parse_->GetNals().size());
}


void CMFCAppDlg::OnBnClickedBtnSave()
{
    if (!nal_parse_)
    {
        MessageBoxW(L"not parse nal.");
        return;
    }

    if (!parse_fininsed_)
    {
        MessageBoxW(L"parsing nal ... ...");
        return;
    }

    if (nal_parse_->GetNals().size() <= 0)
    {
        MessageBoxW(L"nal is empty!... ...");
        return;
    }
    WCHAR buffer[MAX_PATH]{ 0 };
    GetCurrentDirectoryW(MAX_PATH, buffer);
    CFileDialog file_save_dlg(FALSE);
    file_save_dlg.m_ofn.lpstrInitialDir = buffer;
    file_save_dlg.m_ofn.lpstrTitle = L"保存文件";
    file_save_dlg.m_ofn.lpstrDefExt = L".";
    if (DecodeFormat::H264 == stream_type_) file_save_dlg.m_ofn.lpstrDefExt = L"h264";
    else if (DecodeFormat::HEVC == stream_type_) file_save_dlg.m_ofn.lpstrDefExt = L"hevc";
    file_save_dlg.m_ofn.lpstrFilter = L"(*.*)\0\0";

    if (IDOK == file_save_dlg.DoModal())
    {
        auto file_path = w2c(file_save_dlg.GetPathName().GetBuffer());
        std::ofstream dump_file_stream(file_path, std::ios::binary);
        if (!dump_file_stream.is_open())
        {
            MessageBoxW(L"save file failed");
            return;
        }
        nal_parse_->DumpFile(dump_file_stream);
        dump_file_stream.close();
    }

}


void CMFCAppDlg::OnCustomdrawListNalView(NMHDR* pNMHDR, LRESULT* pResult)
{
    *pResult = CDRF_DODEFAULT;
    LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
    // TODO: 在此添加控件通知处理程序代码
    NMLVCUSTOMDRAW* lplvdr = (NMLVCUSTOMDRAW*)pNMHDR;
    NMCUSTOMDRAW& nmcd = lplvdr->nmcd;

    switch (lplvdr->nmcd.dwDrawStage)
    {
    case CDDS_PREPAINT:
        *pResult = CDRF_NOTIFYITEMDRAW;
        break;
    case CDDS_ITEMPREPAINT:
    {
        auto iter = items_colors_.find(nmcd.dwItemSpec);
        if (iter != items_colors_.end())
        {
            lplvdr->clrTextBk = iter->second;

            *pResult = CDRF_DODEFAULT;
        }
    }
    break;
    default:
        break;
    }



}


void CMFCAppDlg::OnBnClickedCancel()
{
    // TODO: 在此添加控件通知处理程序代码
   // CDialogEx::OnCancel();
}


void CMFCAppDlg::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
    CDialogEx::OnWindowPosChanging(lpwndpos);

    // TODO: 在此处添加消息处理程序代码
}


void CMFCAppDlg::OnClickListNalView(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    // TODO: 在此添加控件通知处理程序代码
    auto item = pNMItemActivate->iItem;
    if (item >= 0)
    {
        std::string nal_data;
        std::string nal_hex;
        if (nal_parse_)
        {
            nal_data = nal_parse_->GetNalData(item);

            nal_data = nal::NalParse::ToUpper(nal::NalParse::Bin2HexString(nal_data));
            auto size = nal_data.size();
            nal_hex.resize(3 * size);
            for (auto index = 0, index_hex = 0; index < size; index += 2, index_hex += 3)
            {
                nal_hex[index_hex] = nal_data[index];
                nal_hex[index_hex + 1] = nal_data[index + 1];
                nal_hex[index_hex + 2] = ' ';
            }
        }

        nal_data_hex_.SetWindowTextW(c2w(nal_hex).c_str());
    }
    *pResult = 0;
}
