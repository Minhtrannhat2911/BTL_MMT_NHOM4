import React, { useState } from 'react';
import { FaRegUser } from 'react-icons/fa';
import { IoHome } from "react-icons/io5";
import { useNavigate } from 'react-router-dom';
import { CiCircleList } from "react-icons/ci";
import { GiExitDoor } from "react-icons/gi";
import { IoHomeSharp } from "react-icons/io5";
import Logpage from '../Logpage/logpage'; // Import component Logpage

import './Nav.css';

const Nav = () => {
    const navigate = useNavigate();
    const [dropdownVisible, setDropdownVisible] = useState(false);
    const [logVisible, setLogVisible] = useState(false); // State để quản lý việc hiển thị khung log

    const handleLog = (e) => {
        e.preventDefault();
        setLogVisible(!logVisible); // Thay đổi state để hiển thị hoặc ẩn khung log
    };

    const handleInfomation = (e) => {
        e.preventDefault();
        navigate('/infomation');
    };

    const handleHomepage = (e) => {
        e.preventDefault();
        navigate('/home');
    };

    const handleLoginRegister = (e) => {
        e.preventDefault();
        navigate('/');
    };

    const toggleDropdown = () => {
        setDropdownVisible(!dropdownVisible);
    };

    return (
        <div className="top-navigation">
            <div className="setting-button">
            <button onClick={handleHomepage} className="setting-icon"><IoHome /></button>
            </div>
            <div className="log-button">
                <button onClick={handleLog} className="log-icon"><CiCircleList /></button>
                {logVisible && (
                    <div className="log-container">
                        <Logpage />
                    </div>
                )}
            </div>
            <div className="user-profile">
                <button onClick={toggleDropdown}><FaRegUser /></button>
                {dropdownVisible && (
                    <div className="dropdown-menu">
                        <button onClick={handleInfomation}>Edit profile</button>
                        <button onClick={handleLoginRegister}> <GiExitDoor />Exit</button>
                    </div>
                )}
                <div className="status-indicator"></div>
            </div>

        </div>
    );
};

export default Nav;