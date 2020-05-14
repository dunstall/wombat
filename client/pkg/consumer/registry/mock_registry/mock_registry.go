// Code generated by MockGen. DO NOT EDIT.
// Source: registry.go

// Package mock_registry is a generated GoMock package.
package mock_registry

import (
	registry "github.com/dunstall/wombatclient/pkg/consumer/registry"
	gomock "github.com/golang/mock/gomock"
	reflect "reflect"
)

// MockRegistry is a mock of Registry interface
type MockRegistry struct {
	ctrl     *gomock.Controller
	recorder *MockRegistryMockRecorder
}

// MockRegistryMockRecorder is the mock recorder for MockRegistry
type MockRegistryMockRecorder struct {
	mock *MockRegistry
}

// NewMockRegistry creates a new mock instance
func NewMockRegistry(ctrl *gomock.Controller) *MockRegistry {
	mock := &MockRegistry{ctrl: ctrl}
	mock.recorder = &MockRegistryMockRecorder{mock}
	return mock
}

// EXPECT returns an object that allows the caller to indicate expected use
func (m *MockRegistry) EXPECT() *MockRegistryMockRecorder {
	return m.recorder
}

// Create mocks base method
func (m *MockRegistry) Create(path string, data []byte, isEphemeral bool) error {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "Create", path, data, isEphemeral)
	ret0, _ := ret[0].(error)
	return ret0
}

// Create indicates an expected call of Create
func (mr *MockRegistryMockRecorder) Create(path, data, isEphemeral interface{}) *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "Create", reflect.TypeOf((*MockRegistry)(nil).Create), path, data, isEphemeral)
}

// Get mocks base method
func (m *MockRegistry) Get(path string) ([]byte, error) {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "Get", path)
	ret0, _ := ret[0].([]byte)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// Get indicates an expected call of Get
func (mr *MockRegistryMockRecorder) Get(path interface{}) *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "Get", reflect.TypeOf((*MockRegistry)(nil).Get), path)
}

// GetRoot mocks base method
func (m *MockRegistry) GetRoot(path string) ([]string, error) {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "GetRoot", path)
	ret0, _ := ret[0].([]string)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// GetRoot indicates an expected call of GetRoot
func (mr *MockRegistryMockRecorder) GetRoot(path interface{}) *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "GetRoot", reflect.TypeOf((*MockRegistry)(nil).GetRoot), path)
}

// Set mocks base method
func (m *MockRegistry) Set(path string, data []byte) error {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "Set", path, data)
	ret0, _ := ret[0].(error)
	return ret0
}

// Set indicates an expected call of Set
func (mr *MockRegistryMockRecorder) Set(path, data interface{}) *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "Set", reflect.TypeOf((*MockRegistry)(nil).Set), path, data)
}

// Delete mocks base method
func (m *MockRegistry) Delete(path string) error {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "Delete", path)
	ret0, _ := ret[0].(error)
	return ret0
}

// Delete indicates an expected call of Delete
func (mr *MockRegistryMockRecorder) Delete(path interface{}) *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "Delete", reflect.TypeOf((*MockRegistry)(nil).Delete), path)
}

// Watch mocks base method
func (m *MockRegistry) Watch(root string) (<-chan bool, error) {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "Watch", root)
	ret0, _ := ret[0].(<-chan bool)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// Watch indicates an expected call of Watch
func (mr *MockRegistryMockRecorder) Watch(root interface{}) *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "Watch", reflect.TypeOf((*MockRegistry)(nil).Watch), root)
}

// ConnEvents mocks base method
func (m *MockRegistry) ConnEvents() <-chan registry.Event {
	m.ctrl.T.Helper()
	ret := m.ctrl.Call(m, "ConnEvents")
	ret0, _ := ret[0].(<-chan registry.Event)
	return ret0
}

// ConnEvents indicates an expected call of ConnEvents
func (mr *MockRegistryMockRecorder) ConnEvents() *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "ConnEvents", reflect.TypeOf((*MockRegistry)(nil).ConnEvents))
}

// Close mocks base method
func (m *MockRegistry) Close() {
	m.ctrl.T.Helper()
	m.ctrl.Call(m, "Close")
}

// Close indicates an expected call of Close
func (mr *MockRegistryMockRecorder) Close() *gomock.Call {
	mr.mock.ctrl.T.Helper()
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "Close", reflect.TypeOf((*MockRegistry)(nil).Close))
}
