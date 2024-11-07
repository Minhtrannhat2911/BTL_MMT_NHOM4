import React, { useState } from 'react';
import './info.css';
import profileImage from '../../assets/man_4140048.png';
import { useNavigate } from 'react-router-dom';
import Nav from '../Nav/Nav';

const Info = () => {
    const [formData, setFormData] = useState({
      fullName: '',
      nickname: '',
      email: '',
      phoneNumber: '',
      country: '',
      gender: 'Gender',
      ipAddress: ''
  });


  const navigate = useNavigate();

  const handleChange = (e) => {
      const { name, value } = e.target;
      setFormData({ ...formData, [name]: value });
  };

  const handleSubmit = (e) => {
      e.preventDefault();
      const { fullName, nickname, email, phoneNumber, country, gender, ipAddress } = formData;
      
      if (fullName && nickname && email && phoneNumber && country && gender && ipAddress) {
          console.log('Form submitted:', formData);
          navigate('/Home');
      } else {
          alert('Please fill in all fields.');
      }
  };

  return (
    <>
      <Nav />
      <div className="profile-form">
          <div className="profile-box">
          <h2 className="left-aligned">Information</h2>
          <hr className="divider" />
          <div className="profile-header">
            <img src={profileImage} alt="Profile" className="profile-image" />
            <h3>{formData.fullName}</h3>
            <p className="info-text">{formData.email} | {formData.phoneNumber}</p>
            </div>
          <form onSubmit={handleSubmit}>
            <div className="form-group-tripple">
              
                <input 
                  type="text" 
                  name="fullName" 
                  value={formData.fullName} 
                  onChange={handleChange} 
                  placeholder="Full name" 
                />
                 <input 
                type="text" 
                name="nickname" 
                value={formData.nickname} 
                onChange={handleChange} 
                placeholder="Nick name" 
              />
                 
               <div className="form-group-double">
               <select
                  name="country" 
                  value={formData.country} 
                  onChange={handleChange}
                  className={formData.country === "Country" ? "placeholder-shown" : ""}
                >
                  <option value="Country" >Country</option>
                  <option value="America">America</option>
                  <option value="China">China</option>
                  <option value="VietNam">VietNam</option>
                </select>
                <select 
                  name="gender" 
                  value={formData.gender} 
                  onChange={handleChange}
                  className={formData.gender === "Gender" ? "placeholder-shown" : ""}
                >
                  <option value="Gender">Gender</option>
                  <option value="Female">Female</option>
                  <option value="Male">Male</option>
                  <option value="Other">Other</option>
                </select>
              </div>
              </div>
             
              <div className="form-group-tripple">
              <input 
                  type="email" 
                  name="email" 
                  value={formData.email} 
                  onChange={handleChange} 
                  placeholder="Email" 
                />  
                <input 
                  type="text" 
                  name="phoneNumber" 
                  value={formData.phoneNumber} 
                  onChange={handleChange} 
                  placeholder="Phone number" 
                />
              <input 
                  type="text" 
                  name="ipAddress" 
                  value={formData.ipAddress} 
                  onChange={handleChange} 
                  placeholder="IP Address" 
                />
              </div>
             
                
            <button type="submit" className="submit-button">Submit</button>
          </form>
          </div>
      </div>
    </>
  );
};

export default Info;