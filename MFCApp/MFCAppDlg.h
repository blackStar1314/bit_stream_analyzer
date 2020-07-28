
// MFCAppDlg.h : 头文件
//

#pragma once

#include "nal_parse.h"
#include <vector>
#include <map>
// CMFCAppDlg 对话框
class CMFCAppDlg : public CDialogEx
{
    // 构造
public:
    CMFCAppDlg(CWnd* pParent = NULL);   // 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_MFCAPP_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持


// 实现
    void InitListCtrl();
    static DWORD WINAPI ParseNalThread(LPVOID);
    void StartParse();
    void UpdateListCtrl();
    void DeleteItem(int first, int last);
    void SetListCtrlItemColor(int item, COLORREF color);
    void DeleteListCtrlItemColor(int item);
protected:
    void InitEditCtrl();
protected:
    HICON m_hIcon;
    CListCtrl nal_unit_list_ctrl_;
    CString file_path_;
    std::unique_ptr<nal::NalParse> nal_parse_;
    std::atomic_bool parse_fininsed_ = true;
    std::vector<int> checked_items_;
    DecodeFormat stream_type_ = DecodeFormat::NONE;
    std::map<int, COLORREF> items_colors_;
    int list_board_width_ = 0, list_board_heigth_ = 0;
    double list_width_rate_ = 0.0f, list_height_rate_ = 0.0f;
    // 生成的消息映射函数
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnDropFiles(HDROP hDropInfo);
    afx_msg void OnClose();
protected:
    afx_msg LRESULT OnUmFilePathChanged(WPARAM wParam, LPARAM lParam);
public:
    afx_msg void OnBnClickedBtnDelNal();
    afx_msg void OnBnClickedBtnSave();
    afx_msg void OnCustomdrawListNalView(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnBnClickedCancel();
    afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
    afx_msg void OnClickListNalView(NMHDR* pNMHDR, LRESULT* pResult);
    CEdit nal_data_hex_;
};
