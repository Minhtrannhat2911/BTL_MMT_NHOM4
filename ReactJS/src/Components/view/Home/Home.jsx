import axios from 'axios';
import React, { useEffect, useRef, useState } from 'react';
import { FaDownload } from 'react-icons/fa';
import { FaTrashCan } from "react-icons/fa6";
import { ImFolderUpload } from "react-icons/im";
import { IoIosArrowForward } from "react-icons/io";
import { RiHome5Line } from "react-icons/ri";
import { useDispatch } from 'react-redux';
import { setLogClientAction, setLogServerAction, setLogTrackerAction } from '../../../redux/action';
import { api } from '../../api/api';
import Nav from '../Nav/Nav';
import './Home.css';

const Home = () => {
    //  Sample data for the file list
    const [files, setFiles] = useState([]);

/*declare state variables*/

    const [currentPage, setCurrentPage] = useState(1);
    const itemsPerPage = 7;
    const [logClient, setLogClient] = useState([]);
    const [logServer, setLogServer] = useState([]);
    const [logTracker, setLogTracker] = useState([]);
    const [action, setAction] = useState('')
    const [view, setView] = useState('upload');
    const [selectedfile, setSelectedFile] = useState([]);
    const [selectedfiles, setSelectedFiles] = useState([]);
    const [uploadStatus, setUploadStatus] = useState("select");
    const [uploadStatuss, setUploadStatuss] = useState("select");
    const [filePath, setFilePath] = useState("");
    const inputRef = useRef();
    const inputRefs = useRef();
    const dispatch = useDispatch();

/*Funtions*/

    const totalPages = Math.ceil(files.length / itemsPerPage);
    const currentFiles = files.slice((currentPage - 1) * itemsPerPage, currentPage * itemsPerPage);

    const handlePageChange = (page) => {
        setCurrentPage(page);
    };
    const handleNextPage = () => {
        setCurrentPage((prevPage) => Math.min(prevPage + 1, totalPages));
    };

    const handlePreviousPage = () => {
        setCurrentPage((prevPage) => Math.max(prevPage - 1, 1));
    };
    const registerLink =() => {
        setAction(' active')
    };
    const loginLink =() => {
        setAction('')
    };

    const handleFileChange =(event) => {
    if(event.target.files && event.target.files.length > 0){
        console.log(event.target.files);
            setSelectedFile(Array.from(event.target.files));
        }
    };
    const handleFilesChange =(event) => {
    if(event.target.files && event.target.files.length > 0){
        console.log(event.target.files);
            setSelectedFiles(Array.from(event.target.files));
        }
    };

    const onChooseFile = () => {
        inputRef.current.click();
    };
    const onChooseFiles = () => {
        inputRefs.current.click();
    };

{/*-------------------   Up file lên    ------------------------------------------------------------------------------------------------*/}

    const handleUpload = async () => {
        if (uploadStatus === "done") {
            clearFileInput();
            return;
        }
        if (uploadStatuss === "done") {
            clearFilesInput();
            return;
        }
        try {

            // Ping the API endpoint
            const formData = new FormData();
            // formData.append("file", selectedfile[0]);
            if(selectedfile === null || selectedfile.length > 0){
                selectedfile.map((file) => formData.append("files", file)) // Sử dụng tên "files" để gửi một danh sách);
            } else {
                selectedfiles.map((file) => formData.append("files", file));
            }
            console.log(formData.getAll("files"));
    
            const response = await api.post(`upload`, formData.getAll("files"), {
                headers: {
                    'Content-Type': 'multipart/form-data',
                },})
                setLogClient(response.data);
                dispatch(setLogClientAction({
                    title: "Client Log",
                    content: response.data,
                }));
                setTimeout(getLogServer, [1000]);
                getLogTracker();
        } catch (error) {
            console.error('Error uploading file:', error);
            setUploadStatus("error");
        }
    };

    // Lấy log của server 
    async function getLogServer() {
        try{
            const response = await axios.get("http://localhost:8003/log-server", {
                headers: {
                    'Content-Type': 'application/json',
                }
            });
            setLogServer(response.data);
            dispatch(setLogServerAction({
                title: "Server Log",
                content: response.data,
            }))
        }catch(e){
            console.error(e);
        }
    }
    // Lấy log của tracker 
    async function getLogTracker() {
        try{
            const response = await axios.get("http://127.0.1.1:8000/log-tracker", {
                headers: {
                    'Content-Type': 'application/json',
                }
            });
            setLogTracker(response.data);
            dispatch(setLogTrackerAction({
                title: "Tracker Log",
                content: response.data
            }));
        }catch(e){
            console.error(e);
        }
    }
    // Lấy tất cả các file đã được chia sẽ 
    useEffect(() => {
        async function getAllFiles(){
            try{
                const response = await api.get("all-files",{
                    headers: {
                        'Content-Type': 'application/json',  // Đảm bảo gửi đúng header
                    },
                });
                console.log(response);
                function fromBEToFiles(file) {
                    if (file.type === 'directory') {
                        const date = new Date(file.uploadDate);
                        return {
                            name: file.name,
                            type: file.type,
                            size: file.size,
                            date: `${date.getDate()}-${date.getMonth() + 1}-${date.getFullYear()}`,
                            contents: file.contents.map(fromBEToFiles),
                        };
                    } else {
                        const date = new Date(file.uploadDate);
                        return {
                            name: file.name,
                            size: file.size,
                            date: `${date.getDate()}-${date.getMonth() + 1}-${date.getFullYear()}`,
                            type: file.type,
                        };
                    }
                }
                console.log(response.data.map(fromBEToFiles));
                setFiles(response.data.map(fromBEToFiles));
            }catch(e){
                console.error(e);
            }
        }

        getAllFiles();
    }, [])

{/*-------------------   Tải file    ------------------------------------------------------------------------------------------------*/}

    const handleDownload = async (e) => {
        console.log(e);
        try {
            const response = await api.post('download', {file_path: e}, {
                headers: {
                    'Content-Type': 'application/json',
                }
            });
            setLogClient(response.data);
                dispatch(setLogClientAction({
                    title: "Client Log",
                    content: response.data,
                }));
                setTimeout(getLogServer, [1000]);
                getLogTracker();
            // const url = window.URL.createObjectURL(new Blob([response.data]));
            // const link = document.createElement('a');
            // link.href = url;
            // link.setAttribute('download', 'filename.ext'); // Replace with your file name and extension
            // document.body.appendChild(link);
            // link.click();
            // link.remove();
        } catch (error) {
            console.error('Error downloading file:', error);
        }
    };

{/*-------------------   Xóa file     ------------------------------------------------------------------------------------------------*/}

    const handleDelete = async () => {
        try {
            await axios.delete('http://localhost:3000/delete', {
                data: { fileName: selectedfile.name } // Replace with the appropriate data
            });
            clearFileInput();
            console.log('File deleted successfully');
        } catch (error) {
            console.error('Error deleting file:', error);
        }
    };

    /*Clear file input*/
    const clearFileInput = () => {
        setSelectedFile([]);
        setUploadStatus("select");
    };
    const clearFilesInput = () => {
        setSelectedFiles([]);
        setUploadStatuss("select");
    };
    return (
        <>
            <Nav />
            <div className="upload-history-container">

{/*-------------------Chuyển Upload--Download------------------------------------------------------------------------------------------------*/}

                <div className="sidebar">
                    <button className="sidebar-btn" onClick={() => setView('upload')}>Upload File</button>
                    <button className="sidebar-btn" onClick={() => setView('download')}>Download File</button>
                </div>

                {/* Main Content */}
                <div className="main-content">

{/*-------------------Trang upload files-----------------------------------------------------------------------------------------------------*/}

                    {view === 'upload' ? (
                        <>
                            <h2><RiHome5Line className="icon1"/><IoIosArrowForward className="icon2"/>Upload File </h2>
                            {/* File Table */}
                            <div className="table-container">
                                <div className="file-upload-container">
                                    <div className="folder-icon"><ImFolderUpload /></div>
                                    <div className="file-upload">Upload File History</div>
                                    <div className="file-uploading"> 
                                        <input
                                            type="file"
                                            ref={inputRefs}
                                            onChange={handleFilesChange}
                                            webkitdirectory="true"
                                            multiple
                                            style={{display: 'none'}}
                                        />
                                        {selectedfiles.length === 0  && (
                                            <button className="file-btn" onClick={onChooseFiles}>
                                                Upload Folder
                                            </button>
                                        )}
                                        {selectedfiles.length > 0 && (
                                            <>
                                                <div className="file-upload-info">
                                                    <div style={{flex:1}}>
                                                        <h6>{selectedfiles.name}</h6>
                                                        <button className="upload-cancel" onClick={clearFilesInput}>X</button>
                                                        <button className="upload-btn" onClick={handleUpload}>
                                                            Upload
                                                        </button>
                                                    </div>
                                                </div>
                                            </>
                                        )}
                                    </div>
                                    <div className="file-uploading1"> 
                                        <input
                                            type="file"
                                            ref={inputRef}
                                            onChange={handleFileChange}
                                            style={{display: 'none'}}
                                        />
                                        {selectedfile.length === 0 && (
                                            <button className="file-btn1" onClick={onChooseFile}>
                                                Upload File
                                            </button>
                                        )}
                                        {selectedfile.length > 0 && (
                                            <>
                                                <div className="file-upload-info1">
                                                    <div style={{flex:1}}>
                                                        <h6>{selectedfile.name}</h6>
                                                        <button className="upload-cancel1" onClick={clearFileInput}>X</button>
                                                        <button className="upload-btn1" onClick={handleUpload}>
                                                            Upload
                                                        </button>
                                                    </div>
                                                </div>
                                            </>
                                        )}
                                    </div>
                                </div>
                                <hr className="border1" />
                                <table className="file-table">
                                    <thead className="table-header">
                                        <tr>
                                            <th className="header-item file-name">File Name</th>
                                            <th className="header-item file-size">File Size</th>
                                            <th className="header-item file-date">File Date</th>
                                            <th className="header-item action">Action</th>
                                        </tr>
                                    </thead>
                                    <tbody>
                                        {
                                            currentFiles.map((file, index) => (
                                                <tr key={index}>
                                                    <td className="file-name">{file.name}</td>
                                                    <td className="file-size">{file.size}</td>
                                                    <td className="file-date">{file.date}</td>
                                                    <td className="action">
                                                    <div onclick={() => handleDelete(file)} className="delete-icon">
                                                        <FaTrashCan style={{ color: 'red' }} />
                                                        </div>

                                               </td>
                                                </tr>
                                        ))}
                                    </tbody>
                                    <div className="pagination-controls">
                                        <button onClick={handlePreviousPage} disabled={currentPage === 1}>Previous</button>
                                        <span>Page {currentPage} of {totalPages}</span>
                                        <button onClick={handleNextPage} disabled={currentPage === totalPages}>Next</button>
                                    </div>
                                </table>
                            </div>
                        </>
                    ) : (
                        <>

{/*-------------------Trang download files-----------------------------------------------------------------------------------------------------*/}

                            <h2><RiHome5Line className="icon1"/><IoIosArrowForward className="icon2"/>Download File </h2>
                            {/* File Table */}
                            <div className="table-container">
                                <div className="file-upload-container">
                                    <div className="folder-icon"><ImFolderUpload /></div>
                                    <div className="file-upload">Download File History</div>
                                </div>
                                <hr className="border1"/>
                                <table className="file-table">
                                    <thead className="table-header">
                                        <tr>
                                            <th className="header-item file-name">File Name</th>
                                            <th className="header-item file-size">File Size</th>
                                            <th className="header-item file-date">File Date</th>
                                            <th className="header-item action">Action</th>
                                        </tr>
                                    </thead>
                                    <tbody>
                                        {
                                            currentFiles.map((file, index) => (
                                                <tr key={index}>
                                                    <td className="file-name">{file.name}</td>
                                                    <td className="file-size">{file.size}</td>
                                                    <td className="file-date">{file.date}</td>
                                                    <td className="action">
                                                    <div onClick={() => handleDownload(file.name)}><FaDownload className="download-icon" /></div>
                                                    </td>
                                                </tr>
                                        ))}
                                    </tbody>
                                    <div className="pagination-controls">
                                        <button onClick={handlePreviousPage} disabled={currentPage === 1}>Previous</button>
                                        <span>Page {currentPage} of {totalPages}</span>
                                        <button onClick={handleNextPage} disabled={currentPage === totalPages}>Next</button>
                                    </div>
                                </table>
                            </div>
                        </>
                    )}
                </div>
            </div>
        </>
    );
};

export default Home;
