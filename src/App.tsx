import { useState, useEffect } from "react";
import * as XLSX from 'xlsx'; 

function App() {
  const [peopleIn, setPeopleIn] = useState(0);
  const [peopleOut, setPeopleOut] = useState(0);
  const [peopleInside, setPeopleInside] = useState(0);
  const [lastUpdateTime, setLastUpdateTime] = useState("Belum ada data");
  const [error, setError] = useState("");

  const CHANNEL_ID = "CHANNEL_ID"; 
  const READ_API_KEY = "READ_API_KEY"; 

  const getThingSpeakUrl = (fieldNum: any) =>
    `https://api.thingspeak.com/channels/${CHANNEL_ID}/fields/${fieldNum}.json?api_key=${READ_API_KEY}&results=1`;

  useEffect(() => {
    const fetchData = async () => {
      setError("");
      try {
        const responseIn = await fetch(getThingSpeakUrl(1));
        const dataIn = await responseIn.json();
        setPeopleIn(
          parseInt(
            dataIn.feeds && dataIn.feeds[0] ? dataIn.feeds[0].field1 : 0
          ) || 0
        );

        const responseOut = await fetch(getThingSpeakUrl(2));
        const dataOut = await responseOut.json();
        setPeopleOut(
          parseInt(
            dataOut.feeds && dataOut.feeds[0] ? dataOut.feeds[0].field2 : 0
          ) || 0
        );

        const responseInside = await fetch(getThingSpeakUrl(3));
        const dataInside = await responseInside.json();
        setPeopleInside(
          parseInt(
            dataInside.feeds && dataInside.feeds[0]
              ? dataInside.feeds[0].field3
              : 0
          ) || 0
        );

        setLastUpdateTime(new Date().toLocaleTimeString());
      } catch (err) {
        console.error("Error fetching data from ThingSpeak:", err);
        setError(
          "Gagal mengambil data. Pastikan API Key dan Channel ID benar, serta ESP32 online."
        );
      }
    };

    fetchData(); 
    const intervalId = setInterval(fetchData);
    return () => clearInterval(intervalId);
  }, [CHANNEL_ID, READ_API_KEY]);

  const exportToExcel = () => {
    const dataToExport = [
      {
        'Total Masuk': peopleIn,
        'Total Keluar': peopleOut,
        'Total Yang ada di dalam': peopleInside,
        'Jumlah Pengunjung Hari Ini': peopleIn, // Sesuai permintaan
        'Waktu Ekspor': new Date().toLocaleString()
      }
    ];

    const ws = XLSX.utils.json_to_sheet(dataToExport);
    const wb = XLSX.utils.book_new();
    XLSX.utils.book_append_sheet(wb, ws, "Data Penghitung Orang");

    const today = new Date();
    const fileName = `Data_Penghitung_Orang_${today.getFullYear()}-${(today.getMonth() + 1).toString().padStart(2, '0')}-${today.getDate().toString().padStart(2, '0')}.xlsx`;

    XLSX.writeFile(wb, fileName);
  };

  return (
    <div className="min-h-screen bg-gradient-to-br from-blue-100 to-purple-100 flex items-center justify-center p-4 font-inter">
      <div className="bg-white rounded-2xl shadow-xl p-8 md:p-12 w-full max-w-4xl border border-gray-200">
        <h1 className="text-4xl md:text-5xl font-extrabold text-center text-gray-800 mb-8 tracking-tight">
          Dashboard Penghitung Orang
        </h1>

        {error && (
          <div className="text-center text-red-600 text-lg mb-4 p-3 bg-red-50 rounded-lg border border-red-200">
            {error}
          </div>
        )}

        <div className="grid grid-cols-1 md:grid-cols-3 gap-6 mb-8">
          <div className="bg-green-50 border border-green-200 rounded-xl p-6 flex flex-col items-center justify-center shadow-md hover:shadow-lg transition-shadow duration-300">
            <h2 className="text-xl font-semibold text-green-700 mb-3">
              Total Masuk
            </h2>
            <p className="text-6xl font-bold text-green-600">{peopleIn}</p>
          </div>

    
          <div className="bg-red-50 border border-red-200 rounded-xl p-6 flex flex-col items-center justify-center shadow-md hover:shadow-lg transition-shadow duration-300">
            <h2 className="text-xl font-semibold text-red-700 mb-3">
              Total Keluar
            </h2>
            <p className="text-6xl font-bold text-red-600">{peopleOut}</p>
          </div>

          <div className="bg-blue-50 border border-blue-200 rounded-xl p-6 flex flex-col items-center justify-center shadow-md hover:shadow-lg transition-shadow duration-300">
            <h2 className="text-xl font-semibold text-blue-700 mb-3">
              Orang di Dalam
            </h2>
            <p className="text-6xl font-bold text-blue-600">{peopleInside}</p>
          </div>
        </div>

        <div className="text-center text-gray-500 text-sm mt-6">
          <p>
            {new Date().toLocaleDateString()} <br />
            <span className="font-medium text-gray-700">{lastUpdateTime}</span>
          </p>
          <p className="text-xs mt-1">
            *Data diambil dari ThingSpeak setiap 15 detik.
          </p>
        </div>
        
        <div className="mt-8 text-center">
          <button
            onClick={exportToExcel}
            className="bg-green-500 hover:bg-green-600 text-white font-bold py-3 px-6 rounded-lg shadow-lg hover:shadow-xl transition-all duration-300 focus:outline-none focus:ring-2 focus:ring-green-400 focus:ring-opacity-75"
          >
            Export Data ke Excel
          </button>
        </div>
      </div>
    </div>
  );
}

export default App;
