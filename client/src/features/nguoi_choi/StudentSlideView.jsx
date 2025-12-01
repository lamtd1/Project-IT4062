import React from "react";
import Button from "../../components/common/Button.jsx";

import myPdfFile from "../../lib/slide.pdf";

const StudentSlideView = () => {
  const pdfSource = myPdfFile;

  return (
    <div className="h-[calc(100vh-140px)] flex flex-col bg-gray-900 rounded-xl overflow-hidden shadow-2xl">
      <div className="bg-white p-4 border-b flex justify-between items-center">
        <h3 className="font-bold text-gray-800">授業スライド：第1課</h3>
        <Button variant="secondary" className="text-xs py-1">
          閉じる
        </Button>
      </div>
      
      {/* Khu vực hiển thị PDF */}
      <div className="flex-1 relative bg-gray-200">
        <iframe 
            src={pdfSource}
            className="w-full h-full border-none"
            title="Lesson Slide"
        />

        {/* Các nút điều hướng (Overlay lên PDF) */}
        <button className="absolute left-4 top-1/2 -translate-y-1/2 p-2 bg-black/30 hover:bg-black/60 rounded-full text-white cursor-pointer flex items-center justify-center transition-colors">
          <span className="material-symbols-outlined">chevron_left</span>
        </button>

        <button className="absolute right-4 top-1/2 -translate-y-1/2 p-2 bg-black/30 hover:bg-black/60 rounded-full text-white cursor-pointer flex items-center justify-center transition-colors">
          <span className="material-symbols-outlined">chevron_right</span>
        </button>
      </div>
    </div>
  );
};

export default StudentSlideView;